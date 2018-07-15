#include "gazebo_visualization.h"

GazeboVis::GazeboVis(){
	
	_node = std::unique_ptr<ignition::transport::Node> (new ignition::transport::Node);

	// Create the marker message
	_markerMsg = std::unique_ptr<ignition::msgs::Marker> (new ignition::msgs::Marker);
	_markerMsg->set_ns("line");
	_markerMsg->set_type(ignition::msgs::Marker::LINE_STRIP);
	_markerMsg->mutable_material()->mutable_script()->set_name("Gazebo/Green");
	// ignition::msgs::Set(_markerMsg->mutable_pose(),
	//                   ignition::math::Pose3d(0, 0, 0, 0, 0, 0));	
	_markerMsgPoint = std::unique_ptr<ignition::msgs::Marker> (new ignition::msgs::Marker);
	_markerMsgPoint->set_ns("point");
	_markerMsgPoint->set_type(ignition::msgs::Marker::SPHERE);
	_markerMsgPoint->mutable_material()->mutable_script()->set_name("Gazebo/Red");
	ignition::msgs::Set(_markerMsgPoint->mutable_scale(),
	                  ignition::math::Vector3d(0.05, 0.05, 0.05));

	_markerMsgTree = std::unique_ptr<ignition::msgs::Marker> (new ignition::msgs::Marker);
	_markerMsgTree->set_ns("OcTree");
	_markerMsgTree->set_type(ignition::msgs::Marker::BOX);
	_markerMsgTree->mutable_material()->mutable_script()->set_name("Gazebo/Green");
	ignition::msgs::Set(_markerMsgTree->mutable_scale(),
	                  ignition::math::Vector3d(0.15, 0.15, 0.15));

	_prev_id_line = 0;
	_prev_id_point = 0;
	_prev_id_tree = 0;
}

bool GazeboVis::addLine(std::vector<std::tuple<double, double, double>>& points){

	_markerMsg->clear_point();
	_markerMsg->set_action(ignition::msgs::Marker::ADD_MODIFY);
	_markerMsg->set_id(_prev_id_line);
	_prev_id_line++;
	for(auto point : points)
	{
		addPoint(std::get<0>(point), std::get<1>(point), std::get<2>(point));
		ignition::msgs::Set(_markerMsg->add_point(),
		    ignition::math::Vector3d(std::get<0>(point), std::get<1>(point), std::get<2>(point)));
	}
	bool success = _node->Request("/marker", *_markerMsg.get());
	gazebo::common::Time::MSleep(1);
	return success;
}

bool GazeboVis::addPoint(double x, double y, double z){

	_markerMsgPoint->set_action(ignition::msgs::Marker::ADD_MODIFY);
	_markerMsgPoint->set_id(_prev_id_point);
	_prev_id_point++;
	// DBG("ID: " << _prev_id_point << " Pos: " << x << " " << y << " " << z);
	ignition::msgs::Set(_markerMsgPoint->mutable_pose(),
	                    ignition::math::Pose3d(x, y, z, 0, 0, 0));
	return _node->Request("/marker", *_markerMsgPoint.get());
}

bool GazeboVis::clearAll(){

	_markerMsg->set_action(ignition::msgs::Marker::DELETE_ALL);
	_markerMsgPoint->set_action(ignition::msgs::Marker::DELETE_ALL);
	_markerMsgTree->set_action(ignition::msgs::Marker::DELETE_ALL);

	if(_node->Request("/marker", *_markerMsg.get()) && _node->Request("/marker", *_markerMsgPoint.get()) && _node->Request("/marker", *_markerMsgTree.get()))
	{
		_prev_id_line = 0;
		_prev_id_point = 0;
		_prev_id_tree = 0;
		return true;
	}
	else
		return false;
}

bool GazeboVis::clearPreviousLine(){
	
	_markerMsg->set_id(_prev_id_line - 1);	
	_markerMsg->set_action(ignition::msgs::Marker::DELETE_MARKER);
	return _node->Request("/marker", *_markerMsg.get());
}

bool GazeboVis::clearPreviousPoint(){
	
	_markerMsg->set_id(_prev_id_point - 1);	
	_markerMsg->set_action(ignition::msgs::Marker::DELETE_MARKER);
	return _node->Request("/marker", *_markerMsgPoint.get());
}

void GazeboVis::clearOctree(){
	_markerMsgTree->set_action(ignition::msgs::Marker::DELETE_MARKER);
	_node->Request("/marker", *_markerMsgTree.get());
	_prev_id_tree = 0;
}
void GazeboVis::visOctree(octomap::OcTree& octree)
{
	_markerMsgTree->set_action(ignition::msgs::Marker::ADD_MODIFY);

	for(octomap::OcTree::tree_iterator it = octree.begin_tree(octree.getTreeDepth()); it!= octree.end_tree(); ++it) 
	{
		if (it.isLeaf() && octree.isNodeOccupied(*it) && octree.isNodeAtThreshold(*it))
		{
			_markerMsgTree->set_id(_prev_id_tree);
			_prev_id_tree++;
			ignition::msgs::Set(_markerMsgTree->mutable_scale(),
			                  ignition::math::Vector3d(it.getSize(), it.getSize(), it.getSize()));
			ignition::msgs::Set(_markerMsgTree->mutable_pose(),
			                  ignition::math::Pose3d(it.getX(), it.getY(), it.getZ(), 0, 0, 0));
			_node->Request("/marker", *_markerMsgTree.get());
			gazebo::common::Time::MSleep(0.1);
	    }
	}
}