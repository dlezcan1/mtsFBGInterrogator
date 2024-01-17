#include "mtsFBGSensor/mtsFBGSensor/mtsFBGSensor.h"
#include "mtsFBGSensor/mtsFBGTool/mtsFBGTool.h"

#include <memory>

#include <cisstCommon/cmnUnits.h>
#include <cisstMultiTask.h>

#include <cisst_ros_bridge/mtsROSBridge.h>

int main(int argc, char* argv[])
{
    ros::init(argc, argv, "fbg_force_tool", ros::init_options::AnonymousName);
    
    mtsFBGSensor fbgSensorTask("FBGSensorTask");
    mtsFBGTool   fbgToolTask("FBGToolTask");

    mtsComponentManager* manager = mtsManagerLocal::GetInstance();

    fbgSensorTask.Configure("192.168.1.101"); // FIXME: update to JSON file/argument
    fbgToolTask.Configure(""); // FIXME: update to FBG tool's JSON config file

    manager->AddComponent(&fbgSensorTask);
    manager->AddComponent(&fbgToolTask);

    ros::NodeHandle fbgNode("fbg_tool");
    mtsROSBridge rosBridge("fbg_tool_ros_bridge", 5.0 * cmn_ms, &fbgNode);
    rosBridge.PerformsSpin(true);

    //
    rosBridge.AddPublisherFromCommandRead<vctDoubleVec, cisst_msgs::vctDoubleVec>(
        "RequiresFBGSensor",
        "GetFBGPeaksState",
        "sensor/measured_wavelengths"
    );

    // rosBridge.AddPublisherFromCommandRead<vctDoubleVec, geometry_msgs::Wrench>(
    //     "RequiresFBGTool",
    //     "GetMeasuredCartesianForces",
    //     "tool/measured_forces_cf"
    // );

    // rosBridge.AddPublisherFromCommandRead<vctDoubleVec, geometry_msgs::Wrench>(
    //     "RequiresFBGTool",
    //     "GetMeasuredCartesianForcesTip",
    //     "tool/measured_forces_tip_cf"
    // );

    // rosBridge.AddPublisherFromCommandRead<vctDoubleVec, geometry_msgs::Wrench>(
    //     "RequiresFBGTool",
    //     "GetMeasuredCartesianForcesSclera",
    //     "tool/measured_forces_sclera_cf"
    // );

    // rosBridge.AddPublisherFromCommandRead<mtsStdString, std_msgs::String>(
    //     "RequiresFBGTool",
    //     "GetToolName",
    //     "tool/name"
    // );

    // Connect components
    manager->AddComponent(&rosBridge);

    manager->Connect(
        rosBridge.GetName(),     "RequiresFBGSensor",
        fbgSensorTask.GetName(), "ProvidesFBGSensor"
    );

    manager->Connect(
        rosBridge.GetName(),   "RequiresFBGTool",
        fbgToolTask.GetName(), "ProvidesFBGTool"
    );

    // Spin 
    manager->CreateAllAndWait(2.0 * cmn_s);
    manager->StartAllAndWait(2.0 * cmn_s);

    ros::spin();
    manager->KillAllAndWait(2.0 * cmn_s);
    manager->Cleanup();

    cmnLogger::Kill();

    return 0;

}