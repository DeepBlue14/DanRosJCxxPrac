#DanRosJCxxPrac
*ROS Android NDK practice (For Dan Brooks' project)*

=====


###**Project Synopsis**
*TBA...*


###**Introduction**

The files which directly relate to the Android/ROS magic are:
- MainActivity.java (launches the ros node and a data comm thread
- JRos.java *(Java container class for elegently launching the ROS node)*
- JRosComm *(Java thread which sends data from Android to ROS)*
- chatter_bot.cpp *(ROS chatter node)*


###**Build & Run**

- Download the ROS binary form http://wiki.ros.org/android_ndk/Tutorials/Building%20The%20Example%20Applications%20using%20the%20Binary%20Distribution (or crosscompile it yourself).  place the roscpp_android_ndk at the same location as this file, and unzip it.
- open chatter_bot.cpp, and set the ROS_MASTER_URI and ROS_IP to the correct ones.

Open 3 terminals.  Run each of these commands in a different terminal:
```
roscore
rostopic echo /a_chatter
rostopic pub /chatter std_msgs/String "Hello!" -r 1
```

Start the app
