/**
Copyright (c) 2020 Somesh Daga <s2daga@uwaterloo.ca>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "uwrt_arm_hw/arm_hw_real.h"
#include "uwrt_arm_hw/can_config.h"

#include <ros/ros.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <string>
#include <sys/ioctl.h>
#include <sys/socket.h>

namespace uwrt
{
namespace arm
{
ArmHWReal::ArmHWReal(const std::string& urdf_str)
  : ArmHWReal("arm_hw_real", urdf_str)
{}

ArmHWReal::ArmHWReal(const std::string& name, const std::string& urdf_str)
  : ArmHW(name, urdf_str),
    use_any_can_device_(false)
{}

bool ArmHWReal::init(ros::NodeHandle& nh,
                     ros::NodeHandle& arm_hw_nh)
{
  if (!ArmHW::init(nh, arm_hw_nh))
    return false;

  // === Initialize CAN Communications ===
  // Get the CAN Device(s) to use for arm control/feedback
  if (!use_any_can_device_)
  {
    if (!arm_hw_nh_.getParam("can_dev", can_device_))
    {
      ROS_FATAL_NAMED("arm_hw_real",
                      "[ArmHWReal] ROS Parameter 'can_dev' not found! "
                      "Failed to initialize the Arm Interface!");
      return false;
    }
  }
  // TODO(someshdaga): Consider allowing communications over all
  //                   existing CAN devices on the PC (e.g. can0 and can1)
  else
  {
    ROS_FATAL_NAMED("arm_hw_real",
                    "[ArmHWReal] Usage of 'any' CAN devices is unsupported. "
                    "Failed to initialize the Arm Interface!");
    return false;
  }

  // Initialize SocketCAN and required filters
  // TODO(someshdaga): Consider usage of CAN_BCM instead of CAN_RAW
  //                   for advanced CAN features (e.g. throttling, cycling etc)
  can_socket_handle_ = socket(PF_CAN, SOCK_DGRAM, CAN_RAW);
  snprintf(can_ifr_.ifr_name, can_device_.length() + 1,
           "%s", can_device_.c_str());
  ioctl(can_socket_handle_, SIOCGIFINDEX, &can_ifr_);
  can_address_.can_family = AF_CAN;
  can_address_.can_ifindex = can_ifr_.ifr_ifindex;
  // Set filter(s) for CAN socket
  struct can_filter filter[8];
  // Filter based on the CAN IDs associated with PC Arm Feedback
  //    - Match Condition: <received_can_id> & mask == can_id & mask
  filter[0].can_id = static_cast<canid_t>(can_id::Get::ARM_ERROR);
  filter[0].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
  filter[1].can_id = can_id::Get::TURNTABLE_POSITION;
  filter[1].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
  filter[2].can_id = can_id::Get::SHOULDER_POSITION;
  filter[2].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
  filter[3].can_id = can_id::Get::ELBOW_POSITION;
  filter[3].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
  filter[4].can_id = can_id::Get::WRIST_PITCH_POSITION;
  filter[4].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
  filter[5].can_id = can_id::Get::WRIST_ROLL_POSITION;
  filter[5].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
  filter[6].can_id = can_id::Get::CLAW_POSITION;
  filter[6].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);
  filter[7].can_id = can_id::Get::FORCE_SENSOR_VALUE;
  filter[7].can_mask = (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_SFF_MASK);

  setsockopt(can_socket_handle_, SOL_CAN_RAW, CAN_RAW_FILTER,
             &filter, sizeof(filter));
  // Lastly, bind the CAN socket to the CAN address
  bind(can_socket_handle_, (struct sockaddr *)&can_address_, sizeof(can_address_));

  ROS_INFO_NAMED("arm_hw_real",
                 "[ArmHWReal] Successfully initialized the Real Arm Interface!");
  return true;
}

void ArmHWReal::read(const ros::Time& time,
                     const ros::Duration& duration)
{
  // TODO(someshdaga): Implement
}

void ArmHWReal::write(const ros::Time& time,
                      const ros::Duration& duration)
{
  // TODO(someshdaga): Implement
}

}  // namespace arm
}  // namespace uwrt