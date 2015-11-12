#include "ros/ros.h"
#include "ros/time.h"
#include "sensor_msgs/JointState.h"
#include "std_msgs/Float64.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h> 

#define PI 3.14
#define g 9.81


//  global parameters



ros::Publisher j0_pub_;
ros::Publisher j1_pub_;
ros::Publisher j2_pub_;
ros::Publisher j3_pub_;
ros::Publisher j4_pub_;
ros::Publisher j5_pub_;
ros::Publisher j6_pub_;
ros::Publisher hight_pub;

int change=0;
double global_z=0;

double j0_pos_public,j1_pos_public,j2_pos_public,j3_pos_public,j4_pos_public,j5_pos_public,j6_pos_public;
double j0_vel_public,j1_vel_public,j2_vel_public,j3_vel_public,j4_vel_public,j5_vel_public,j6_vel_public;
typedef struct Task {
	float x;
	float z;
	float psi;
	float dx;
	float dz;
	float dpsi;
};
/////// avishai's code//////
// Links lengths
const float L1 = 420e-3; //Second and third links
const float L2 = 400e-3; //Fourth and fifth links
const float L3 = 235.9e-3; //Sixth link and gripper
Task DirectKinematics3R(float q[3], float dq[3]) {
	//Direct kinematics function
	Task G;

	G.x = L1*sin(q[0]) + L2*sin(q[0]+q[1]) + L3*sin(q[0]+q[1]+q[2]);
	G.z = L1*cos(q[0]) + L2*cos(q[0]+q[1]) + L3*cos(q[0]+q[1]+q[2]);
	G.psi = q[0]+q[1]+q[2]; // pitch is relative to the vertical axis z

	G.dx = L1*cos(q[0])*dq[0] + L2*cos(q[0]+q[1])*(dq[0]+dq[1]) + L3*cos(q[0]+q[1]+q[2])*(dq[0]+dq[1]+dq[2]);
	G.dz = L1*sin(q[0])*dq[0] + L2*sin(q[0]+q[1])*(dq[0]+dq[1]) + L3*sin(q[0]+q[1]+q[2])*(dq[0]+dq[1]+dq[2]);
	G.dpsi = dq[0]+dq[1]+dq[2]; 

	return G;
}

void InverseKinematics3R(Task G, float* q, float* dq) {
	//Inverse kinematics function
	float xt, zt, gamma, R;

	xt = G.x - L3*sin(G.psi);
	zt = G.z - L3*cos(G.psi);
	gamma = atan2(zt, xt);
	R = sqrt(xt*xt + zt*zt);
	
	q[0] = PI/2 - (gamma + acos((R*R+L1*L1-L2*L2)/(2*L1*R)));
	q[1] = PI/2 - (atan2(zt-L1*cos(q[0]), xt-L1*sin(q[0])) + q[0]);
	q[2] = G.psi - q[0] - q[1];

	dq[0] = (G.dx*sin(q[0] + q[1]) - G.dz*cos(q[0] + q[1]) + L3*G.dpsi*sin(q[2]))/(L1*sin(q[1]));
	dq[1] = -(L2*G.dx*sin(q[0] + q[1]) - L2*G.dz*cos(q[0] + q[1]) - L1*G.dz*cos(q[0]) + L1*G.dx*sin(q[0]) + L1*L3*G.dpsi*sin(q[1] + q[2]) + L2*L3*G.dpsi*sin(q[2]))/(L1*L2*sin(q[1]));
	dq[2] = (G.dx*sin(q[0]) - G.dz*cos(q[0]) + L3*G.dpsi*sin(q[1] + q[2]) + L2*G.dpsi*sin(q[1]))/(L2*sin(q[1]));
}

float** TrajectoryGenerator(int N, float T, float xc, float z0, float vd) {
	// N - number of trajectory points
	// T - duratiohight_pubn of motion
	// xc - constant x coordinate to move along
	// z0 - initial z coordinate
	// vd - desired velocity after T 
	// motion along: z(t) = vd/(2T)*t^2 + z0

	int i, j;
	float dt = T/N, t = 0;
	Task G;
	float q[3];
	float dq[3];

	// Allocate trajectory matrix
	float** angles = (float**)malloc(N*sizeof(float*));
	for (i=0; i<N; i++)
		angles[i] = (float*)malloc(6*sizeof(float));

	G.x = xc; G.dx = 0;
	G.psi = PI/2; G.dpsi = 0;
	for (i=0; i<N; i++) {
		t += dt;
		G.z = vd/(2*T)*t*t + z0;
		G.dz = vd/T*t;
		InverseKinematics3R(G, q, dq);
		for (j=0; j<3; j++) {
			angles[i][j] = q[j];
			angles[i][j+3] = dq[j];
		}
	}
	return angles;
}

float VelocityCalc(float z) {
	// z - desired height increase
	// return the desired throw velocity
	return sqrt(2*g*z);
}

///////end of avishai's code /////////
void move_arm(double x,double z,double teta,double grip)
{
	float q_inv[3];
	float dq_inv[3];
	Task temp_position;
	  temp_position.x=x;
	  temp_position.z=z;
	  temp_position.psi=teta;

	std_msgs::Float64 j1;
	std_msgs::Float64 j2;
	std_msgs::Float64 j3;
	std_msgs::Float64 right_fingre;
	std_msgs::Float64 left_fingre;

	InverseKinematics3R(temp_position, q_inv,dq_inv);
		
		j1.data=q_inv[0];
		j2.data=q_inv[1];
		j3.data=q_inv[2];
		right_fingre.data=grip;
		left_fingre.data=-grip;
		j0_pub_.publish(j1);
		j2_pub_.publish(j2);
		j4_pub_.publish(j3);
		j5_pub_.publish(left_fingre);
		j6_pub_.publish(right_fingre);
}


void close_arm(double grip)
{
	

	std_msgs::Float64 right_fingre;
	std_msgs::Float64 left_fingre;

		right_fingre.data=grip;
		left_fingre.data=-grip;

		j5_pub_.publish(left_fingre);
		j6_pub_.publish(right_fingre);
}

void jointCallback(const sensor_msgs::JointState &msg)
{
	j0_pos_public = msg.position[0];//jont1
	j1_pos_public = msg.position[1];//joint2
	j2_pos_public = msg.position[2];//joint3
	j3_pos_public = msg.position[3];//right fingre
	j4_pos_public = msg.position[4];//left fingre
}

void z_callback(const std_msgs::Float64::ConstPtr &new_z)
{
 global_z=new_z->data;
   change =1;
  
}

int main(int argc, char **argv)
{
int i, j, N=300;
double jj;
float** angles;
double th_time;
double Vc;
Task current_position, Temp;
 Task temp_position;
  ros::init(argc, argv, "kuka_or");

  ros::NodeHandle n;


	j0_pub_  = n.advertise<std_msgs::Float64>("/kuka/kuka_1_controller/command", 10000);//joint 1
	j2_pub_  = n.advertise<std_msgs::Float64>("/kuka/kuka_3_controller/command", 10000);//joint 2
	j4_pub_  = n.advertise<std_msgs::Float64>("/kuka/kuka_5_controller/command", 10000);//joint 3
	j5_pub_  = n.advertise<std_msgs::Float64>("/kuka/kuka_left_finger_controller/command", 10000); //left fingre
	j6_pub_  = n.advertise<std_msgs::Float64>("/kuka/kuka_right_finger_controller/command", 10000);//right fingre
	hight_pub  = n.advertise<std_msgs::Float64>("/kuka_z", 10000);//right fingre
  ros::Subscriber sub_z = n.subscribe("/position_z", 10000,z_callback);
  ros::Subscriber sub_joint = n.subscribe("/kuka/joint_states", 10000,jointCallback);
  ros::Rate loop_rate(30);
  ros::Duration dt_loop(0.001/N);
  ros::Duration pause(0.15);
  ros::Duration open_time(0.01);

std_msgs::Float64 j1;
std_msgs::Float64 j2;
std_msgs::Float64 j3;
std_msgs::Float64 j4;
std_msgs::Float64 j5;
std_msgs::Float64 j6;
std_msgs::Float64 j7;
std_msgs::Float64 z_to_pub;
int stage=0;
/////// avishai's code ///////









while(ros::ok()) {

float q[]={(float)j0_pos_public,(float)j1_pos_public,(float)j2_pos_public};
float dq[3]={0};
current_position=DirectKinematics3R(q,dq);
Vc=VelocityCalc(global_z);
z_to_pub.data=current_position.z;
hight_pub.publish(z_to_pub);

//ROS_INFO("\nx:%f \nz:%f",current_position.x,current_position.z);

if(stage==0)
{
for (jj=0; jj<100 ; jj++)
{
z_to_pub.data=current_position.z;
hight_pub.publish(z_to_pub);
move_arm(0.3+(jj/300),0.05,1.57,0);
loop_rate.sleep();

ros::spinOnce();
}
stage=1;
}
if(stage==1)
{
for (jj=0; jj<50 ; jj++)
{
z_to_pub.data=current_position.z;
hight_pub.publish(z_to_pub);
move_arm(0.6333+(jj/300),0.05,1.57,-0.5);
loop_rate.sleep();
ros::spinOnce();
}
stage=2;
}
if(stage==2)
{
for (jj=0; jj<50 ; jj++)
{
z_to_pub.data=current_position.z;
hight_pub.publish(z_to_pub);
move_arm(0.8,0.05,1.57,-0.5+(jj/30));
loop_rate.sleep();
ros::spinOnce();
}
stage=3;
}
if(stage==3)
{
for (jj=0; jj<100 ; jj++)
{
z_to_pub.data=current_position.z;
hight_pub.publish(z_to_pub);
move_arm(0.8-(jj/800),0.05+(jj/3000),1.57,10);
loop_rate.sleep();
ros::spinOnce();
}
stage=4;
}
//ROS_INFO("\nteta 1:%f \nteta 2 :%f \nteta 3:%f ",j0_pos_public,j1_pos_public,j2_pos_public);




///////brings the arm to catch the object///////
//ROS_INFO("\nteta 1:%f \nteta 2 :%f \nteta 3:%f \nteta 5 :%f \nteta 6:%f \nstage :%d",j0_pos_public,j1_pos_public,j2_pos_public,j3_pos_public,j4_pos_public,stage);



////////////////////





if(change==1 && stage==4){

	angles =  TrajectoryGenerator(N, 0.0005, current_position.x, current_position.z, Vc*50);
////// moving the arm to get specific velocity/////////

	for (i=0; i<N; i++) {
q[0]=(float)j0_pos_public;
q[1]=(float)j1_pos_public;
q[2]=(float)j2_pos_public;
		j1.data=angles[i][0];
		j2.data=angles[i][1];
		j3.data=angles[i][2];
current_position=DirectKinematics3R(q,dq);
		Temp = DirectKinematics3R(angles[i],dq);
		ROS_INFO("\nz:%f   %f",j0_pos_public,j1.data);
		j0_pub_.publish(j1);
		j2_pub_.publish(j2);
		j4_pub_.publish(j3);
		dt_loop.sleep();
		//ROS_INFO("\z:%f ",current_position.z);
                ros::spinOnce();
current_position=DirectKinematics3R(q,dq);

////
z_to_pub.data=current_position.z;
hight_pub.publish(z_to_pub);
/////
if(i>N-50)
{

close_arm(-10);


}


	}

pause.sleep();
close_arm(100);
////////////////////////////////////////////////////

for(i=0;i<100;i++)
{
z_to_pub.data=current_position.z;
hight_pub.publish(z_to_pub);
open_time.sleep();
close_arm(1-0.011*i);
ros::spinOnce();
}
z_to_pub.data=current_position.z;
hight_pub.publish(z_to_pub);
close_arm(100);

////// opening and closing the arm to change the location of the object/////////
/*
close_arm(-100);
th_time=(Vc-sqrt((pow(Vc,2)-2*g*global_z)))/g;
for (i=0; i<th_time; i++) {
pause.sleep();
 ros::spinOnce();
}
ROS_INFO("\nh_time:%f ",th_time);
ros::spinOnce();
close_arm(2);
*/
ros::spinOnce();
change=0;
}

//////////////////////////////////////////////////////////////////////


   
    

    
    
    
  
		
		ros::spinOnce();
	}


  return 0;
}


