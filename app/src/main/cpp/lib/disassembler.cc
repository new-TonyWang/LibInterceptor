/*
 * Copyright (C) 2017 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "disassembler.h"

#include <utility>
#include "target.h"
#if defined(__arm__)
#include "ARM/target_arm.h"
#elif defined(__arm64__) || defined(__aarch64__)
#include "AArch64/target_aarch64.h"
#elif defined(__i386__)
#include "X86/target_x86.h"
#endif


using namespace interceptor;

Disassembler::Disassembler(llvm::Triple triple) : triple_(std::move(triple)){}

Disassembler *Disassembler::Create(const llvm::Triple &triple) {
  std::unique_ptr<Disassembler> disassembler(new Disassembler(triple));
  if (disassembler->Initialize()) return disassembler.release();
  return nullptr;
}

bool Disassembler::Initialize() {
  std::string error;
  const llvm::Target *target =
      llvm::TargetRegistry::lookupTarget(triple_.str().c_str(), error);
  if (!target) return false;

  sti_.reset(target->createMCSubtargetInfo(triple_.str().c_str(), "", ""));
  if (!sti_) return false;

  mri_.reset(target->createMCRegInfo(triple_.str()));
  if (!mri_) return false;

  ctx_.reset(new llvm::MCContext( nullptr,mri_.get(), nullptr));
  dis_.reset(target->createMCDisassembler(*sti_, *ctx_));
  if (!dis_) return false;

  return true;
}

bool Disassembler::GetInstruction(const void *data, size_t offset,
                                  llvm::MCInst &inst, uint64_t &inst_size) {
  const uint8_t *data_p = static_cast<const uint8_t *>(data);
  llvm::ArrayRef<uint8_t> data_arr(data_p + offset, 32);
  auto status = dis_->getInstruction(inst, inst_size, data_arr, offset,
                                     llvm::nulls(),llvm::nulls());

  switch (status) {
    case llvm::MCDisassembler::Success:
      return true;
    case llvm::MCDisassembler::Fail:
    case llvm::MCDisassembler::SoftFail:
      return false;
  }
  llvm_unreachable("Unexpected decode status");
}
