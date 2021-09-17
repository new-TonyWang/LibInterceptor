package com.tencent.interceptor_lib_enhanced;



import android.app.Activity;
import android.os.Bundle;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class MainActivity extends Activity {
//    static {
//        try {
//
//            Class clazz = MainActivity.class.getClassLoader().loadClass("java.lang.Runtime");
//            Class[] cArg = new Class[1];
//            cArg[0] = java.lang.String.class;
//            Method m = clazz.getDeclaredMethod("load",cArg);
//            m.invoke(clazz.newInstance(),"gpu_interceptor");
//        } catch (ClassNotFoundException | NoSuchMethodException e) {
//            e.printStackTrace();
//        } catch (IllegalAccessException e) {
//            e.printStackTrace();
//        } catch (InvocationTargetException | InstantiationException e) {
//            e.printStackTrace();
//        }
//        Runtime r = Runtime.getRuntime();
//        r.load("gpu_interceptor");
//
//
//        //System.loadLibrary("gpu_interceptor");
//    }
    static {
        System.loadLibrary("gpu_interceptor");
//        System.loadLibrary("testxhook");
}
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);
    }

    @Override
    protected void onResume() {
        super.onResume();
        hook();
    }

    public native void hook();
}