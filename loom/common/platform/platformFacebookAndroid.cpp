/*
 * ===========================================================================
 * Loom SDK
 * Copyright 2011, 2012, 2013
 * The Game Engine Company, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ===========================================================================
 */

#include "loom/common/platform/platform.h"

#if LOOM_ALLOW_FACEBOOK && (LOOM_PLATFORM == LOOM_PLATFORM_ANDROID)

#include <jni.h>
#include "platformAndroidJni.h"

#include "loom/common/core/log.h"
#include "loom/common/core/assert.h"
#include "loom/common/platform/platformFacebook.h"
#include "loom/vendor/jansson/jansson.h"


lmDefineLogGroup(gAndroidFacebookLogGroup, "facebook", 1, LoomLogDefault);


static SessionStatusCallback gSessionStatusCallback = NULL;
static FrictionlessRequestCallback gFrictionlessRequestCallback = NULL;


extern "C"
{
    void Java_co_theengine_loomplayer_LoomFacebook_sessionStatusCallback(JNIEnv* env, jobject thiz, jint sessonState, jstring sessionPermissions, jint errorCode)
    {
        const char *sessionPermissionsString = env->GetStringUTFChars(sessionPermissions, 0);
        if (gSessionStatusCallback)
        {
            gSessionStatusCallback((int)sessonState, sessionPermissionsString, (int)errorCode);
        }
        env->ReleaseStringUTFChars(sessionPermissions, sessionPermissionsString);
    }
    
    void Java_co_theengine_loomplayer_LoomFacebook_frictionlessRequestCallback(JNIEnv* env, jobject thiz, jboolean jSuccess)
    {
        if (gFrictionlessRequestCallback)
        {
            gFrictionlessRequestCallback((bool)jSuccess);
        }
    }
}


static loomJniMethodInfo gIsActive;
static loomJniMethodInfo gOpenSessionReadPermissions;
static loomJniMethodInfo gRequestNewPublishPermissions;
static loomJniMethodInfo gFrictionlessRequestDialog;
static loomJniMethodInfo gGetAccessToken;
static loomJniMethodInfo gCloseTokenInfo;
static loomJniMethodInfo gGetExpirationDate;
static loomJniMethodInfo gIsPermissionGranted;


///initializes the data for the Facebook class for Android
void platform_facebookInitialize(SessionStatusCallback sessionStatusCB, FrictionlessRequestCallback frictionlessRequestCB)
{
    lmLog(gAndroidFacebookLogGroup, "Initializing Facebook for Android");

    gSessionStatusCallback = sessionStatusCB;   
    gFrictionlessRequestCallback = frictionlessRequestCB;   

    // Bind to JNI entry points.
    LoomJni::getStaticMethodInfo(gIsActive,
                                    "co/theengine/loomplayer/LoomFacebook",
                                    "isActive",
                                    "()Z");
    LoomJni::getStaticMethodInfo(gOpenSessionReadPermissions,
                                    "co/theengine/loomplayer/LoomFacebook",
                                    "openSessionWithReadPermissions",
                                    "(Ljava/lang/String;)Z");
    LoomJni::getStaticMethodInfo(gRequestNewPublishPermissions,
                                    "co/theengine/loomplayer/LoomFacebook",
                                    "requestNewPublishPermissions",
                                    "(Ljava/lang/String;)Z");
    LoomJni::getStaticMethodInfo(gFrictionlessRequestDialog,
                                    "co/theengine/loomplayer/LoomFacebook",
                                    "showFrictionlessRequestDialog",
                                    "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
    LoomJni::getStaticMethodInfo(gGetAccessToken,
                                    "co/theengine/loomplayer/LoomFacebook",
                                    "getAccessToken",
                                    "()Ljava/lang/String;");
    LoomJni::getStaticMethodInfo(gCloseTokenInfo,
                                 "co/theengine/loomplayer/LoomFacebook",
                                 "closeAndClearTokenInformation",
                                 "()V");    
    LoomJni::getStaticMethodInfo(gGetExpirationDate,
                                    "co/theengine/loomplayer/LoomFacebook",
                                    "getExpirationDate",
                                    "(Ljava/lang/String;)Ljava/lang/String;");
    LoomJni::getStaticMethodInfo(gIsPermissionGranted,
                                    "co/theengine/loomplayer/LoomFacebook",
                                    "isPermissionGranted",
                                    "(Ljava/lang/String;)Z");
}


bool platform_isFacebookActive()
{
    return gIsActive.getEnv()->CallStaticBooleanMethod(gIsActive.classID, gIsActive.methodID);
}


bool platform_openSessionWithReadPermissions(const char* permissionsString)
{
    jstring jPermissionsString = gOpenSessionReadPermissions.getEnv()->NewStringUTF(permissionsString);
    jboolean result = gOpenSessionReadPermissions.getEnv()->CallStaticBooleanMethod(gOpenSessionReadPermissions.classID,
                                                                                gOpenSessionReadPermissions.methodID,
                                                                                jPermissionsString);
    gOpenSessionReadPermissions.getEnv()->DeleteLocalRef(jPermissionsString);
    return result;
}


bool platform_requestNewPublishPermissions(const char* permissionsString)
{
    jstring jPermissionsString = gRequestNewPublishPermissions.getEnv()->NewStringUTF(permissionsString);
    jboolean result = gRequestNewPublishPermissions.getEnv()->CallStaticBooleanMethod(gRequestNewPublishPermissions.classID,
                                                                                    gRequestNewPublishPermissions.methodID,
                                                                                    jPermissionsString);
    gRequestNewPublishPermissions.getEnv()->DeleteLocalRef(jPermissionsString);
    return result;
}


void platform_showFrictionlessRequestDialog(const char* recipientsString, const char* titleString, const char* messageString)
{
    jstring jRecipientsString = gFrictionlessRequestDialog.getEnv()->NewStringUTF(recipientsString);
    jstring jTitleString = gFrictionlessRequestDialog.getEnv()->NewStringUTF(titleString);
    jstring jMessageString = gFrictionlessRequestDialog.getEnv()->NewStringUTF(messageString);

    gFrictionlessRequestDialog.getEnv()->CallStaticVoidMethod(gFrictionlessRequestDialog.classID,
                                                            gFrictionlessRequestDialog.methodID,
                                                            jRecipientsString,
                                                            jTitleString,
                                                            jMessageString);
    gFrictionlessRequestDialog.getEnv()->DeleteLocalRef(jRecipientsString);
    gFrictionlessRequestDialog.getEnv()->DeleteLocalRef(jTitleString);
    gFrictionlessRequestDialog.getEnv()->DeleteLocalRef(jMessageString);
}


const char* platform_getAccessToken()
{
    static char accessTokenStatic[1024];

    jstring result = (jstring)gGetAccessToken.getEnv()->CallStaticObjectMethod(gGetAccessToken.classID,
                                                                               gGetAccessToken.methodID);
    strncpy(accessTokenStatic, LoomJni::jstring2string(result).c_str(), 1024);

    gGetAccessToken.getEnv()->DeleteLocalRef(result);

    return accessTokenStatic;
}


void platform_closeAndClearTokenInformation()
{
    gCloseTokenInfo.getEnv()->CallStaticVoidMethod(gCloseTokenInfo.classID, gCloseTokenInfo.methodID);
}


const char* platform_getExpirationDate(const char* dateFormat)
{
    static char expirationDateStatic[1024];

    jstring jdateFormatString   = gGetExpirationDate.getEnv()->NewStringUTF(dateFormat);

    jstring result = (jstring)gGetExpirationDate.getEnv()->CallStaticObjectMethod(gGetExpirationDate.classID,
                                                                                gGetExpirationDate.methodID,
                                                                                jdateFormatString);
    strncpy(expirationDateStatic, LoomJni::jstring2string(result).c_str(), 1024);

    gGetExpirationDate.getEnv()->DeleteLocalRef(result);
    return expirationDateStatic;
}

bool platform_isPermissionGranted(const char* permission)
{
    jstring jPermission = gIsPermissionGranted.getEnv()->NewStringUTF(permission);
    jboolean result = gIsPermissionGranted.getEnv()->CallStaticBooleanMethod(gIsPermissionGranted.classID,
                                                                                gIsPermissionGranted.methodID,
                                                                                jPermission);
    gIsPermissionGranted.getEnv()->DeleteLocalRef(jPermission);
    return result;
}

#endif
