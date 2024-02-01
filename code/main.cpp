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
    
    // command line options
    cmnCommandLineOptions options;
    std::string jsonFBGSensorConfigFile;
    std::string jsonFBGToolConfigFile;

    options.AddOptionOneValue(
        "s", "json-config-sensor",
        "The FBG Sensor/interrogator's configuration",
        cmnCommandLineOptions::REQUIRED_OPTION,
        &jsonFBGSensorConfigFile
    );

    options.AddOptionOneValue(
        "t", "json-config-tool",
        "The FBG Tool's configuration",
        cmnCommandLineOptions::REQUIRED_OPTION,
        &jsonFBGToolConfigFile
    );

    // Configure tasks
    fbgSensorTask.Configure(jsonFBGSensorConfigFile);
    fbgToolTask.Configure(jsonFBGToolConfigFile); // FIXME: update to FBG tool's JSON config file

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

    rosBridge.AddPublisherFromCommandRead<vctDoubleVec, cisst_msgs::vctDoubleVec>(
        "RequiresFBGTool",
        "GetMeasuredVectorCartesianForces",
        "tool/deprecated/measured_forces_cf"
    ); // THIS IS DEPRECATED: FROM PREVIOUS IMPLEMENTATION. UPDATING TO vct6 -> geometry_msgs::Wrench

    rosBridge.AddPublisherFromCommandRead<vctDoubleVec, cisst_msgs::vctDoubleVec>(
        "RequiresFBGTool",
        "GetMeasuredVectorCartesianForcesTip",
        "tool/deprecated/measured_forces_tip_cf"
    ); // THIS IS DEPRECATED: FROM PREVIOUS IMPLEMENTATION. UPDATING TO vct6 -> geometry_msgs::Wrench

    rosBridge.AddPublisherFromCommandRead<vctDoubleVec, cisst_msgs::vctDoubleVec>(
        "RequiresFBGTool",
        "GetMeasuredVectorCartesianForcesSclera",
        "tool/deprecated/measured_forces_sclera_cf"
    ); // THIS IS DEPRECATED: FROM PREVIOUS IMPLEMENTATION. UPDATING TO vct6 -> geometry_msgs::Wrench


    rosBridge.AddPublisherFromCommandRead<vct6, geometry_msgs::WrenchStamped>(
        "RequiresFBGTool",
        "GetMeasuredCartesianForcesTip",
        "tool/measured_forces_tip_cf"
    );

    rosBridge.AddPublisherFromCommandRead<vct6, geometry_msgs::WrenchStamped>(
        "RequiresFBGTool",
        "GetMeasuredCartesianForcesSclera",
        "tool/measured_forces_sclera_cf"
    );

    rosBridge.AddPublisherFromCommandRead<mtsStdString, std_msgs::String>(
        "RequiresFBGTool",
        "GetToolName",
        "tool/name"
    );

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