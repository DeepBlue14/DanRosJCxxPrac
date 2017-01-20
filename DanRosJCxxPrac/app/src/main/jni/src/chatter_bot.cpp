/**
 * File: test.cpp
 * Author: James Kuczynski <jkuczyns@cs.uml.edu>
 * File Description: Simple "chatter" ROS node which publishes a message every time it receives
 *                   one.
 * Created: 01/03/2017
 * Last Modified: 01/17/2017
 */

// Android-specific lib used for converting between Java/C++ data types (i.e. Java String --> jstring --> std::string
#include <jni.h>
#include <android/log.h>

// C++ headers
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <fstream>

// ROS headers
#include <ros/ros.h>
#include <std_msgs/String.h>


std::string msgToPub = "hello ROS!"; /// The message to publish
int loopCount = 0; /// global counter to keep track of msgs received
ros::Publisher chatter_pub; /// ROS publisher
/**
 * Reference to the JavaVM to get access to Java function tables (so we can call Java functions
 * from C++) by accessing the JNI environment (JNIEnv).  A reference to the JNIEnv should NOT
 * be saved directly, since Java maintains ownership of this data type.  Thus, the Java garbage
 * collector can delete it even if the C++-side is still using it.
 */
static JavaVM* jvm;
jobject m_obj; /// Reference to the instance of the JCxxComm class so we can call its methods.

//destroy check vars
typedef struct JMethod
{
    bool isLookupFirstCheck = true; //check to see if we have already queried the method
    jclass clazz; // Reference to the Java class
    jmethodID methodId; // Reference to the Java class method
} JMethod;
JMethod destroyJMeth; // Data to use the destroyRequested Java method from C++
JMethod updateUiJMeth; // Data to use the updateUi Java method from C++


/**
 * Use the Android logging system from C++
 */
void log(const char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    __android_log_vprint(ANDROID_LOG_INFO, "ROSCPP_NDK_EXAMPLE", msg, args);
    va_end(args);
}


bool destroyRequested()
{
    /*
     * Call Java method to update the UI.
     * TODO: Should the look-up be done only once?
     */

    //---------------------------------------------------
    JNIEnv* env = new JNIEnv();

    jint rs = jvm->AttachCurrentThread(&env, NULL);
    (rs == JNI_OK) ? log("SUCCESSFULLY passed JNIEnv ref") : log("FAILED to pass JNIEnv reff");

    if(destroyJMeth.isLookupFirstCheck)
    {
        destroyJMeth.clazz = env->FindClass("edu/uml/cs/danrosjcxxprac/UniversalDat");
        destroyJMeth.methodId = env->GetStaticMethodID(destroyJMeth.clazz, "destroyRequested", "()Z");
        if(destroyJMeth.methodId == 0)
            log("ERROR: attempting to get methodID");
        destroyJMeth.isLookupFirstCheck = false;
    }


    jboolean response = env->CallStaticBooleanMethod(destroyJMeth.clazz, destroyJMeth.methodId);
    bool desRec = (jboolean) response;
    //---------------------------------------------------

    return desRec;
}


/**
 * Regular roscpp callback function.
 */
void callback(const std_msgs::StringConstPtr& msg)
{
    ROS_INFO("%s", msg->data.c_str() );

    /*
     * Call Java method to update the UI.
     * TODO: Should the look-up be done only once?
     */
    //---------------------------------------------------
    JNIEnv* env = new JNIEnv();

    jint rs = jvm->AttachCurrentThread(&env, NULL);
    (rs == JNI_OK) ? log("SUCCESSFULLY passed JNIEnv ref") : log("FAILED to pass JNIEnv reff");

    jstring jstr = env->NewStringUTF(msg->data.c_str() );

    if(updateUiJMeth.isLookupFirstCheck)
    {
        updateUiJMeth.clazz = env->FindClass("edu/uml/cs/danrosjcxxprac/JCxxComm");
        updateUiJMeth.methodId = env->GetMethodID(updateUiJMeth.clazz, "updateUi", "(Ljava/lang/String;)V");
    }
    //class clazz = env->FindClass("edu/uml/cs/danrosjcxxprac/JCxxComm");
    //jmethodID messageMe = env->GetMethodID(clazz, "updateUi", "(Ljava/lang/String;)V");
    env->CallVoidMethod(m_obj, updateUiJMeth.methodId, jstr);
    //---------------------------------------------------

    // Regular ROS sub/pub stuff
    std_msgs::String msgo;
    std::stringstream ss;
    ss << msgToPub.c_str() << loopCount++;
    msgo.data = ss.str();
    chatter_pub.publish(msgo);
    log(msg->data.c_str());
}


/**
 * main function of this node.
 */
int main()
{
    int argc = 3;
    // TODO: don't hardcode ip addresses
    char* argv[] = {const_cast<char*>("nothing_important"),
                    const_cast<char*>("__master:=http://robot-brain2:11311"),
                    const_cast<char*>("__ip:=10.10.10.184")}; //10.0.7.145

    ros::init(argc, &argv[0], "android_ndk_native_cpp");

    std::string master_uri = ros::master::getURI();

    (ros::master::check() ) ? log("found rosmaster: ") : log("FAILED to find rosmaster: ");
    log(master_uri.c_str() );

    // Create nodehandle, subscribers and publishers
    ros::NodeHandle n;
    chatter_pub = n.advertise<std_msgs::String>("a_chatter", 1000);
    ros::Subscriber sub = n.subscribe("chatter", 1000, callback);
    ros::WallRate loop_rate(100);

    // Check if ros is fine, and if the (Java) main activity's onDestroy() method.
    while(ros::ok() && !destroyRequested() )
    {
        ros::spinOnce();
        loop_rate.sleep();
    }

    return EXIT_SUCCESS;
}


#ifdef __cplusplus
    extern "C" {
#endif

/**
 * Mutator function to update the message to publish
 * @see #msgToPub
 */
void Java_edu_uml_cs_danrosjcxxprac_JCxxComm_setMsgToPub(JNIEnv* env, jobject /*this*/, jstring msg)
{
    const char* nativeString = (*env).GetStringUTFChars(msg, (jboolean *) false);
    std::string tmp(nativeString);
    msgToPub = std::string(nativeString);
    (*env).ReleaseStringUTFChars(msg, nativeString);
}


/**
 * Passes references to the environmnet (for function look-up table stuff) and the instance of
 * JCxxComm.
 */
void Java_edu_uml_cs_danrosjcxxprac_JCxxComm_init(JNIEnv* env, jobject obj)
{
    jint rs = env->GetJavaVM(&jvm);
    (rs == JNI_OK) ? log("JVM ref successfully passed to C") : log("FAILED to pass JVM ref");

    m_obj = reinterpret_cast<jobject>(env->NewGlobalRef(obj) );
}


/**
 * Initiate the regular ROS code by calling the C++ thread's main function
 */
void Java_edu_uml_cs_danrosjcxxprac_JRos_startRosNode(JNIEnv* env, jobject obj)
{
    jint rs = env->GetJavaVM(&jvm);
    (rs == JNI_OK) ? log("JVM ref successfully passed to C") : log("FAILED to pass JVM ref");

    m_obj = reinterpret_cast<jobject>(env->NewGlobalRef(obj) );
    main();
}


#ifdef __cplusplus
    }
#endif