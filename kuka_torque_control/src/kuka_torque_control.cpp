#include "ros/ros.h"
#include "ros/time.h"
#include "sensor_msgs/JointState.h"
#include "std_msgs/Float64.h"

//  global parameters


ros::Publisher j0_pub_;
ros::Publisher j1_pub_;
ros::Publisher j2_pub_;
ros::Publisher j3_pub_;
ros::Publisher j4_pub_;
ros::Publisher j5_pub_;
ros::Publisher j6_pub_;

const int max_effort = 176;

double j0_pos,j1_pos,j2_pos,j3_pos,j4_pos,j5_pos,j6_pos;
double j0_vel,j1_vel,j2_vel,j3_vel,j4_vel,j5_vel,j6_vel;


void send_effort_commands()
{
	std_msgs::Float64 eff_cmd;

	ros::Time time_now = ros::Time::now();

	eff_cmd.data = max_effort*cos(1*time_now.sec);
	j0_pub_.publish(eff_cmd);

	eff_cmd.data = max_effort*sin(1*time_now.sec);
	j1_pub_.publish(eff_cmd);

	eff_cmd.data = max_effort*cos(1*time_now.sec);
	j2_pub_.publish(eff_cmd);

	eff_cmd.data = max_effort*cos(1*time_now.sec);
	j3_pub_.publish(eff_cmd);

	eff_cmd.data = max_effort*cos(1*time_now.sec);
	j4_pub_.publish(eff_cmd);

	eff_cmd.data = max_effort*cos(1*time_now.sec);
	j5_pub_.publish(eff_cmd);

	eff_cmd.data = max_effort*cos(1*time_now.sec);
	j6_pub_.publish(eff_cmd);

	// wait 0.01 seconds -> 100Hz
	ros::Duration(0.01).sleep();

}

void jointCallback(const sensor_msgs::JointState &msg)
{
	j0_pos = msg.position[0];
	j1_pos = msg.position[1];
	j2_pos = msg.position[2];
	j3_pos = msg.position[3];
	j4_pos = msg.position[4];
	j5_pos = msg.position[5];
	j6_pos = msg.position[6];

	j0_vel = msg.velocity[0];
	j1_vel = msg.velocity[1];
	j2_vel = msg.velocity[2];
	j3_vel = msg.velocity[3];
	j4_vel = msg.velocity[4];
	j5_vel = msg.velocity[5];
	j6_vel = msg.velocity[6];

	ROS_INFO("I heard: [j0 j1 j2 j3 j4 j5 j6] = [ %f %f %f %f %f %f %f ]",j0_pos,j1_pos,j2_pos,j3_pos,j4_pos,j5_pos,j6_pos);

}

int main(int argc, char **argv)
{
	ros::init(argc, argv, "move_arm");
	ros::NodeHandle n;

	j0_pub_  = n.advertise<std_msgs::Float64>("/kuka/joint0_effort_controller/command", 100);
	j1_pub_  = n.advertise<std_msgs::Float64>("/kuka/joint1_effort_controller/command", 100);
	j2_pub_  = n.advertise<std_msgs::Float64>("/kuka/joint2_effort_controller/command", 100);
	j3_pub_  = n.advertise<std_msgs::Float64>("/kuka/joint3_effort_controller/command", 100);
	j4_pub_  = n.advertise<std_msgs::Float64>("/kuka/joint4_effort_controller/command", 100);
	j5_pub_  = n.advertise<std_msgs::Float64>("/kuka/joint5_effort_controller/command", 100);
	j6_pub_  = n.advertise<std_msgs::Float64>("/kuka/joint6_effort_controller/command", 100);

    ros::Subscriber 		joints_sub_ = n.subscribe("/kuka/joint_states", 100 , jointCallback );

	while(ros::ok()) {
		send_effort_commands();
		ros::spinOnce();
	}

	return 0;
}
