/*
 * Copyright (c) 2013-2014 Chukong Technologies Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#include <android/log.h>
#include "CCJavascriptJavaBridge.h"
#include "cocos2d.h"
#include "platform/android/jni/JniHelper.h"
#include "spidermonkey_specifics.h"
#include "ScriptingCore.h"
#include "js_manual_conversions.h"

#define  LOG_TAG    "CCJavascriptJavaBridge"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL Java_org_cocos2dx_lib_Cocos2dxJavascriptJavaBridge_evalString
  (JNIEnv *env, jclass cls, jstring value)
{
    const char *_value = env->GetStringUTFChars(value, NULL);
    ScriptingCore::getInstance()->evalString(_value,NULL);
    env->ReleaseStringUTFChars(value, _value);
    
    return 1;
}

#ifdef __cplusplus
}
#endif

JavascriptJavaBridge::CallInfo::~CallInfo(void)
{
    if (m_returnType == TypeString && m_ret.stringValue)
    {
        delete m_ret.stringValue;
    }
}

bool JavascriptJavaBridge::CallInfo::execute(void)
{
    switch (m_returnType)
    {
        case TypeVoid:
            m_env->CallStaticVoidMethod(m_classID, m_methodID);
            break;

        case TypeInteger:
            m_ret.intValue = m_env->CallStaticIntMethod(m_classID, m_methodID);
            break;

        case TypeFloat:
            m_ret.floatValue = m_env->CallStaticFloatMethod(m_classID, m_methodID);
            break;

        case TypeBoolean:
            m_ret.boolValue = m_env->CallStaticBooleanMethod(m_classID, m_methodID);
            break;

        case TypeString:
            m_retjstring = (jstring)m_env->CallStaticObjectMethod(m_classID, m_methodID);
            const char *stringBuff = m_env->GetStringUTFChars(m_retjstring, 0);
            m_ret.stringValue = new string(stringBuff);
            m_env->ReleaseStringUTFChars(m_retjstring, stringBuff);
           break;
    }

    if (m_env->ExceptionCheck() == JNI_TRUE)
    {
        m_env->ExceptionDescribe();
        m_env->ExceptionClear();
        m_error = JSJ_ERR_EXCEPTION_OCCURRED;
        return false;
    }

    return true;
}


bool JavascriptJavaBridge::CallInfo::executeWithArgs(jvalue *args)
{
    switch (m_returnType)
     {
         case TypeVoid:
             m_env->CallStaticVoidMethodA(m_classID, m_methodID, args);
             break;

         case TypeInteger:
             m_ret.intValue = m_env->CallStaticIntMethodA(m_classID, m_methodID, args);
             break;

         case TypeFloat:
             m_ret.floatValue = m_env->CallStaticFloatMethodA(m_classID, m_methodID, args);
             break;

         case TypeBoolean:
             m_ret.boolValue = m_env->CallStaticBooleanMethodA(m_classID, m_methodID, args);
             break;

         case TypeString:
             m_retjstring = (jstring)m_env->CallStaticObjectMethodA(m_classID, m_methodID, args);
             const char *stringBuff = m_env->GetStringUTFChars(m_retjstring, 0);
             m_ret.stringValue = new string(stringBuff);
             m_env->ReleaseStringUTFChars(m_retjstring, stringBuff);
            break;
     }

    if (m_env->ExceptionCheck() == JNI_TRUE)
    {
        m_env->ExceptionDescribe();
        m_env->ExceptionClear();
        m_error = JSJ_ERR_EXCEPTION_OCCURRED;
        return false;
    }

    return true;
}

bool JavascriptJavaBridge::CallInfo::validateMethodSig()
{
    size_t len = m_methodSig.length();
    if (len < 3 || m_methodSig[0] != '(') // min sig is "()V"
    {
        m_error = JSJ_ERR_INVALID_SIGNATURES;
        return false;
    }

    size_t pos = 1;
    while (pos < len && m_methodSig[pos] != ')')
    {
        JavascriptJavaBridge::ValueType type = checkType(m_methodSig, &pos);
        if (type == TypeInvalid) return false;

        m_argumentsCount++;
        m_argumentsType.push_back(type);
        pos++;
    }

    if (pos >= len || m_methodSig[pos] != ')')
    {
        m_error = JSJ_ERR_INVALID_SIGNATURES;
        return false;
    }

    pos++;
    m_returnType = checkType(m_methodSig, &pos);
    return true;
}

JavascriptJavaBridge::ValueType JavascriptJavaBridge::CallInfo::checkType(const string& sig, size_t *pos)
{
    switch (sig[*pos])
    {
        case 'I':
            return TypeInteger;
        case 'F':
            return TypeFloat;
        case 'Z':
            return TypeBoolean;
        case 'V':
            return TypeVoid;
        case 'L':
            size_t pos2 = sig.find_first_of(';', *pos + 1);
            if (pos2 == string::npos)
            {
                m_error = JSJ_ERR_INVALID_SIGNATURES;
                return TypeInvalid;
            }

            const string t = sig.substr(*pos, pos2 - *pos + 1);
            if (t.compare("Ljava/lang/String;") == 0)
            {
                *pos = pos2;
                return TypeString;
            }
            else if (t.compare("Ljava/util/Vector;") == 0)
            {
                *pos = pos2;
                return TypeVector;
            }
            else
            {
                m_error = JSJ_ERR_TYPE_NOT_SUPPORT;
                return TypeInvalid;
            }
    }

    m_error = JSJ_ERR_TYPE_NOT_SUPPORT;
    return TypeInvalid;
}


bool JavascriptJavaBridge::CallInfo::getMethodInfo(void)
{
    m_methodID = 0;
    m_env = 0;

    JavaVM* jvm = cocos2d::JniHelper::getJavaVM();
    jint ret = jvm->GetEnv((void**)&m_env, JNI_VERSION_1_4);
    switch (ret) {
        case JNI_OK:
            break;

        case JNI_EDETACHED :
            if (jvm->AttachCurrentThread(&m_env, NULL) < 0)
            {
                LOGD("%s", "Failed to get the environment using AttachCurrentThread()");
                m_error = JSJ_ERR_VM_THREAD_DETACHED;
                return false;
            }
            break;

        case JNI_EVERSION :
        default :
            LOGD("%s", "Failed to get the environment using GetEnv()");
            m_error = JSJ_ERR_VM_FAILURE;
            return false;
    }
    jstring _jstrClassName = m_env->NewStringUTF(m_className.c_str());
    m_classID = (jclass) m_env->CallObjectMethod(cocos2d::JniHelper::classloader,
                                                   cocos2d::JniHelper::loadclassMethod_methodID,
                                                   _jstrClassName);

    if (NULL == m_classID) {
        LOGD("Classloader failed to find class of %s", m_className.c_str());
    }

    m_env->DeleteLocalRef(_jstrClassName);
    m_methodID = m_env->GetStaticMethodID(m_classID, m_methodName.c_str(), m_methodSig.c_str());
    if (!m_methodID)
    {
        m_env->ExceptionClear();
        LOGD("Failed to find method id of %s.%s %s",
                m_className.c_str(),
                m_methodName.c_str(),
                m_methodSig.c_str());
        m_error = JSJ_ERR_METHOD_NOT_FOUND;
        return false;
    }

    return true;
}

JS::Value JavascriptJavaBridge::convertReturnValue(JSContext *cx, ReturnValue retValue, ValueType type)
{
	JS::Value ret = JSVAL_NULL;

	switch (type)
	{
		case TypeInteger:
			return INT_TO_JSVAL(retValue.intValue);
		case TypeFloat:
			return DOUBLE_TO_JSVAL((double)retValue.floatValue);
		case TypeBoolean:
			return BOOLEAN_TO_JSVAL(retValue.boolValue);
		case TypeString:
			return c_string_to_jsval(cx, retValue.stringValue->c_str(),retValue.stringValue->size());
	}

	return ret;
}

/**
 *  @brief Initialize Object and needed properties.
 *
 */
JS_BINDED_CLASS_GLUE_IMPL(JavascriptJavaBridge);

/**
 *  @brief Implementation for the Javascript Constructor
 *
 */
JS_BINDED_CONSTRUCTOR_IMPL(JavascriptJavaBridge)
{
    JavascriptJavaBridge* jsj = new JavascriptJavaBridge();

    js_proxy_t *p;
    jsval out;
    
    JSObject *obj = JS_NewObject(cx, &JavascriptJavaBridge::js_class, JavascriptJavaBridge::js_proto, JavascriptJavaBridge::js_parent);
    
    if (obj) {
        JS_SetPrivate(obj, jsj);
        out = OBJECT_TO_JSVAL(obj);
    }

    JS_SET_RVAL(cx, vp, out);
    p =jsb_new_proxy(jsj, obj);
    
    JS_AddNamedObjectRoot(cx, &p->obj, "JavascriptJavaBridge");
    return true;
}

/**
 *  @brief destructor for Javascript
 *
 */
static void basic_object_finalize(JSFreeOp *freeOp, JSObject *obj)
{
    CCLOG("jsbindings: finalizing JS object %p (JavascriptJavaBridge)", obj);
    JavascriptJavaBridge* native = (JavascriptJavaBridge*)JS_GetPrivate(obj);
    if(native)
    {
        delete native;
    }
}

/**
 *  @brief implementation for call java static method
 */
JS_BINDED_FUNC_IMPL(JavascriptJavaBridge, callStaticMethod)
{
    jsval *argv = JS_ARGV(cx, vp);
    if (argc == 3) {
        JSStringWrapper arg0(argv[0]);
        JSStringWrapper arg1(argv[1]);
        JSStringWrapper arg2(argv[2]);

        CallInfo call(arg0.get(), arg1.get(), arg2.get());
        if(call.isValid()){
            bool success = call.execute();
            int errorCode = call.getErrorCode();
            if(errorCode < 0)
                JS_ReportError(cx, "js_cocos2dx_JSJavaBridge : call result code: %d", errorCode);
            JS_SET_RVAL(cx, vp, convertReturnValue(cx, call.getReturnValue(), call.getReturnValueType()));
            return success;	
        }
    }
    else if(argc > 3){
        JSStringWrapper arg0(argv[0]);
        JSStringWrapper arg1(argv[1]);
        JSStringWrapper arg2(argv[2]);
        
        CallInfo call(arg0.get(), arg1.get(), arg2.get());
        if(call.isValid() && call.getArgumentsCount() == (argc - 3)){
            int count = argc - 3;
            jvalue *args = new jvalue[count];
            for (int i = 0; i < count; ++i){
                int index = i + 3;
                switch (call.argumentTypeAtIndex(i))
                {
                    case TypeInteger:
                        double interger;
                        JS::ToNumber(cx, JS::RootedValue(cx, argv[index]), &interger);
                        args[i].i = (int)interger;
                        break;

                    case TypeFloat:
                        double floatNumber;
                        JS::ToNumber(cx, JS::RootedValue(cx, argv[index]), &floatNumber);
                        args[i].f = (float)floatNumber;
                        break;

                    case TypeBoolean:
                        args[i].z = JS::ToBoolean(JS::RootedValue(cx, argv[index])) ? JNI_TRUE : JNI_FALSE;
                        break;

                    case TypeString:
                    default:
                        JSStringWrapper arg(argv[index]);
                        args[i].l = call.getEnv()->NewStringUTF(arg.get());
                        break;
                }
            }
            bool success = call.executeWithArgs(args);
            if (args) delete []args;
            int errorCode = call.getErrorCode();
            if(errorCode < 0)
                JS_ReportError(cx, "js_cocos2dx_JSJavaBridge : call result code: %d", errorCode);
            JS_SET_RVAL(cx, vp, convertReturnValue(cx, call.getReturnValue(), call.getReturnValueType()));
            return success;
        }
                
    }else{
    	JS_ReportError(cx, "js_cocos2dx_JSJavaBridge : wrong number of arguments: %d, was expecting more than 3", argc);	
    }
    
    return false;
}

static bool js_is_native_obj(JSContext *cx, JS::HandleObject obj, JS::HandleId id, JS::MutableHandleValue vp)
{
    vp.set(BOOLEAN_TO_JSVAL(true));
    return true;    
}

/**
 *  @brief register JavascriptJavaBridge to be usable in js
 *
 */
void JavascriptJavaBridge::_js_register(JSContext *cx, JSObject *global)
{
    JSClass jsclass = {
        "JavascriptJavaBridge", JSCLASS_HAS_PRIVATE, JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub, JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub,basic_object_finalize, JSCLASS_NO_OPTIONAL_MEMBERS
    };
    
    JavascriptJavaBridge::js_class = jsclass;
    static JSPropertySpec props[] = {
        {"__nativeObj", 0, JSPROP_ENUMERATE | JSPROP_PERMANENT, JSOP_WRAPPER(js_is_native_obj), JSOP_NULLWRAPPER},
        {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
    };
    
    static JSFunctionSpec funcs[] = {
        JS_BINDED_FUNC_FOR_DEF(JavascriptJavaBridge, callStaticMethod),
        JS_FS_END
    };
    
    JavascriptJavaBridge::js_parent = NULL;
    JavascriptJavaBridge::js_proto = JS_InitClass(cx, global, NULL, &JavascriptJavaBridge::js_class , JavascriptJavaBridge::_js_constructor, 0, props, funcs, NULL, NULL);
}