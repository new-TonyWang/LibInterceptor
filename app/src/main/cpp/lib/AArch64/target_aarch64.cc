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

#include "target_aarch64.h"
#include "disassembler.h"
#include <cassert>
#include <memory>

#include "MCTargetDesc/AArch64MCTargetDesc.h"
#include "llvm/ADT/Triple.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstBuilder.h"

#include "code_generator.h"
#include "disassembler.h"

using namespace interceptor;

static llvm::Triple GetTriple() {
    llvm::Triple triple(llvm::sys::getProcessTriple());
    assert(triple.getArch() == llvm::Triple::aarch64 &&
           "Invalid default host triple for target");
    return triple;
}

CodeGenerator *TargetAARCH64::GetCodeGenerator(void *address,
                                               size_t start_alignment) {
    return CodeGenerator::Create(GetTriple(), start_alignment);
}

Disassembler *TargetAARCH64::CreateDisassembler(void *address) {
    return Disassembler::Create(GetTriple());
}

std::vector<TrampolineConfig> TargetAARCH64::GetTrampolineConfigs(
        uintptr_t start_address) const {
    std::vector<TrampolineConfig> configs;
    configs.push_back({FIRST_4G_TRAMPOLINE, false, 0x10000, 0xffffffff});
    configs.push_back({FULL_TRAMPOLINE, false, 0, 0xffffffffffffffff});
    return configs;
}

/**
 * 生成跳到新函数的跳板
 * @param config
 * @param codegen
 * @param source 旧函数(未使用)
 * @param target 新函数
 * @return
 */
Error TargetAARCH64::EmitTrampoline(const TrampolineConfig &config,
                                    CodeGenerator &codegen, void *source,
                                    void *target) {
    switch (config.type) {
        case FIRST_4G_TRAMPOLINE: {
            uint64_t target_addr = reinterpret_cast<uintptr_t>(target);
            if (target_addr > 0xffffffff)
                return Error("Target address is out of range for the trampoline");
            uint32_t target_addr32 = target_addr;
            codegen.AddInstruction(//增加指令
                    llvm::MCInstBuilder(llvm::AArch64::LDRWl)//加入opecode
                            .addReg(llvm::AArch64::X17)//x17是内部调用临时寄存器,属于operand
                            .addExpr(
                                    codegen.CreateDataExpr(target_addr32)));//属于operand,但是常量写在了常量池中，
            codegen.AddInstruction(
                    llvm::MCInstBuilder(llvm::AArch64::BR).addReg(llvm::AArch64::X17));
            return Error();
        }
        case FULL_TRAMPOLINE: {
            uint64_t target_addr = reinterpret_cast<uintptr_t>(target);
            codegen.AddInstruction(llvm::MCInstBuilder(llvm::AArch64::LDRXl)
                                           .addReg(llvm::AArch64::X17)
                                           .addExpr(codegen.CreateDataExpr(target_addr)));
            codegen.AddInstruction(
                    llvm::MCInstBuilder(llvm::AArch64::BR).addReg(llvm::AArch64::X17));
            return Error();
        }
    }
    return Error("Unsupported trampoline type");
}

static void *calculatePcRelativeAddress(void *data, size_t pc_offset,
                                        size_t offset, bool page_align) {
    uintptr_t data_addr = reinterpret_cast<uintptr_t>(data);
    assert((data_addr & 3) == 0 && "Unaligned data address");
    assert((pc_offset & 3) == 0 && "Unaligned PC offset");

    data_addr += pc_offset;  // Add the PC
    if (page_align) {
        data_addr &= ~0x0fff;  // Align to 4KB
        offset <<= 12;
    }
    data_addr += offset;  // Add the offset
    //一句话，pc = pc+ 偏移
    return reinterpret_cast<void *>(data_addr);
}

// IP1 (second intra-procedure-call scratch register) is X17 and it is used in
// the trampoline so we need special handling for it.
static bool hasIP1Operand(const llvm::MCInst &inst) {
    for (size_t i = 0; i < inst.getNumOperands(); ++i) {
        const llvm::MCOperand &op = inst.getOperand(i);
        if (op.isReg() && op.getReg() == llvm::AArch64::X17) return true;
    }
    return false;
}

void TargetAARCH64::SkipTrampolineFunction(void *&data, size_t offset,
                                           llvm::MCInst &inst) {
    switch (inst.getOpcode()) {
        case llvm::AArch64::B://最为常见，无返回值的跳转

        case llvm::AArch64::BL: {
            uint64_t imm = inst.getOperand(0).getImm() << 2;

            data = reinterpret_cast<void *> (calculatePcRelativeAddress(data, offset, imm, false));
            break;
        }
        case llvm::AArch64::BR:
        case llvm::AArch64::BLR: {
            unsigned reg = inst.getOperand(0).getReg();
//        asm volatile (
//              "mov "
//                );
            //TODO--解决使用寄存器跳转(使用内联汇编拿取)
            break;
        }
        default: {
            return;
        }
    }

}

bool TargetAARCH64::CheckHook(const llvm::MCInst &inst) {
    unsigned int opcode = inst.getOpcode();
    switch (currentState_) {
        case Target::HEALTHY: {
            if (opcode == llvm::AArch64::LDRWl || opcode == llvm::AArch64::LDRXl) {
                jmpreg_ = inst.getOperand(0);
                currentState_ = DANGEROUS;
            }
            break;
        }
        case Target::DANGEROUS: {
            llvm::MCOperand current_Oprand = inst.getOperand(0);
            switch (opcode) {
              //TODO--使用宏优化代码格式
                case llvm::AArch64::B:
                case llvm::AArch64::BR: {
                    if (jmpreg_.isReg() && current_Oprand.isReg()) {
                        if (jmpreg_.getReg() == current_Oprand.getReg()) {
                            //检测到了hook
                            currentState_ = DEAD;
                            return true;
                        }
                        currentState_ = HEALTHY;
                        return false;
                    }
                    break;
                }
                case llvm::AArch64::BLR:
                case llvm::AArch64::BL: {
                    if (jmpreg_.isImm() && current_Oprand.isImm()) {
                        if (jmpreg_.getImm() == current_Oprand.getImm()) {
                            //检测到了hook
                            currentState_ = DEAD;
                            return true;
                        }
                        currentState_ = HEALTHY;
                        return false;
                    }
                    break;
                }
                default: {
                    currentState_ = HEALTHY;
                    return false;
                }
            }
            break;
        }
        case Target::DEAD:
            //目前不会运行到这里
            break;
    }
    return false;
}


bool TargetAARCH64::EndOfFunction(const llvm::MCInst &inst) {
    switch (inst.getOpcode()) {
        case llvm::AArch64::BR:
        case llvm::AArch64::BLR:
        case llvm::AArch64::B:
        case llvm::AArch64::BL:
        case llvm::AArch64::RET:
            return true;
    }
    return false;
}


Error TargetAARCH64::RewriteInstruction(const llvm::MCInst &inst,
                                        CodeGenerator &codegen, void *data,
                                        size_t offset,
                                        bool &possible_end_of_function) {
    switch (inst.getOpcode()) {
        case llvm::AArch64::ADDXri:
        case llvm::AArch64::ANDXri:
        case llvm::AArch64::LDRXui:
        case llvm::AArch64::LDRWl:
        case llvm::AArch64::MOVNWi:
        case llvm::AArch64::MOVNXi:
        case llvm::AArch64::MOVZWi:
        case llvm::AArch64::MOVZXi:
        case llvm::AArch64::MRS:
        case llvm::AArch64::ORRWrs:
        case llvm::AArch64::ORRXrs:
        case llvm::AArch64::STPDi:
        case llvm::AArch64::STPXi:
        case llvm::AArch64::STPXpre:
        case llvm::AArch64::STRBBui:
        case llvm::AArch64::STRSui:
        case llvm::AArch64::STRWui:
        case llvm::AArch64::STRQui:
        case llvm::AArch64::STRXpre:
        case llvm::AArch64::STRXui:
        case llvm::AArch64::SUBSWri:
        case llvm::AArch64::SUBSXri:

        case llvm::AArch64::SUBXri: {
            if (hasIP1Operand(inst))
                return Error(
                        "Instruction not handled yet when one of the operand is IP1 (%s "
                        "(OpcodeId: %d))",
                        codegen.PrintInstruction(inst).c_str(), inst.getOpcode());
            possible_end_of_function = false;
            codegen.AddInstruction(inst);
            break;
        }
        case llvm::AArch64::ADRP: {//以页为单位读取指令
            uint32_t Rd = inst.getOperand(0).getReg();
            uint64_t imm = inst.getOperand(1).getImm();
            possible_end_of_function = false;

            if (hasIP1Operand(inst))
                return Error(
                        "Instruction not handled yet when one of the operand is IP1 (%s "
                        "(OpcodeId: %d))",
                        codegen.PrintInstruction(inst).c_str(), inst.getOpcode());

            uint64_t addr = reinterpret_cast<uintptr_t>(
                    calculatePcRelativeAddress(data, offset, imm, true));
            codegen.AddInstruction(llvm::MCInstBuilder(llvm::AArch64::LDRXl)
                                           .addReg(Rd)
                                           .addExpr(codegen.CreateDataExpr(addr)));
            break;
        }
        case llvm::AArch64::B: {
            uint64_t imm = inst.getOperand(0).getImm() << 2;
            possible_end_of_function = true;

            uint64_t addr = reinterpret_cast<uintptr_t>(
                    calculatePcRelativeAddress(data, offset, imm, false));
            codegen.AddInstruction(llvm::MCInstBuilder(llvm::AArch64::LDRXl)
                                           .addReg(llvm::AArch64::X17)
                                           .addExpr(codegen.CreateDataExpr(addr)));
            codegen.AddInstruction(
                    llvm::MCInstBuilder(llvm::AArch64::BR).addReg(llvm::AArch64::X17));
            break;
        }
        case llvm::AArch64::BL: {
            uint64_t imm = inst.getOperand(0).getImm() << 2;
            possible_end_of_function = true;

            uint64_t addr = reinterpret_cast<uintptr_t>(
                    calculatePcRelativeAddress(data, offset, imm, false));
            codegen.AddInstruction(llvm::MCInstBuilder(llvm::AArch64::LDRXl)
                                           .addReg(llvm::AArch64::X17)
                                           .addExpr(codegen.CreateDataExpr(addr)));
            codegen.AddInstruction(
                    llvm::MCInstBuilder(llvm::AArch64::BLR).addReg(llvm::AArch64::X17));
            break;
        }
        case llvm::AArch64::CBZX: {//条件跳转
            uint32_t Rt = inst.getOperand(0).getReg();
            uint64_t imm = inst.getOperand(1).getImm() << 2;
            possible_end_of_function = false;

            if (hasIP1Operand(inst))
                return Error(
                        "Instruction not handled yet when one of the operand is IP0 (%s "
                        "(OpcodeId: %d))",
                        codegen.PrintInstruction(inst).c_str(), inst.getOpcode());

            uint64_t addr = reinterpret_cast<uintptr_t>(
                    calculatePcRelativeAddress(data, offset, imm, false));
            codegen.AddInstruction(
                    llvm::MCInstBuilder(llvm::AArch64::CBNZX).addReg(Rt).addImm(12 >> 2));
            codegen.AddInstruction(llvm::MCInstBuilder(llvm::AArch64::LDRXl)
                                           .addReg(llvm::AArch64::X17)
                                           .addExpr(codegen.CreateDataExpr(addr)));
            codegen.AddInstruction(
                    llvm::MCInstBuilder(llvm::AArch64::BR).addReg(llvm::AArch64::X17));
            break;
        }
        default: {
            possible_end_of_function = true;
            return Error("Unhandled instruction: %s (OpcodeId: %d)",
                         codegen.PrintInstruction(inst).c_str(), inst.getOpcode());
        }
    }
    return Error();
}

