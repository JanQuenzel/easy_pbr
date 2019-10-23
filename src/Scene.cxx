#include "easy_pbr/Scene.h"

//c++

//my stuff
#define LOGURU_REPLACE_GLOG 1
#include <loguru.hpp>


//redeclared things here so we can use them from this file even though they are static
std::vector<MeshSharedPtr>  Scene::m_meshes;
std::mutex Scene::m_mesh_mutex;

Scene::Scene()    
{

}

void Scene::show(const std::shared_ptr<Mesh> mesh, const std::string name){

    std::lock_guard<std::mutex> lock(m_mesh_mutex);  // so that accesed to the map are thread safe

    //check if there is already a mesh with the same name 
    bool found=false;
    int idx_found=-1;
    for(size_t i = 0; i < m_meshes.size(); i++){
        if(m_meshes[i]->name==name){
            found=true;
            idx_found=i;
        }
    }

    if(found){
        m_meshes[idx_found]=mesh; //it's a shared ptr so it just gets asigned to this one and the previous one dissapears
        m_meshes[idx_found]->name=name;
        // m_meshes[idx_found]->recalculate_normals();
    }else{
        m_meshes.push_back(mesh);
        m_meshes.back()->name=name;
        if(m_meshes.back()->V.rows()!=m_meshes.back()->NV.rows()){
            m_meshes.back()->recalculate_normals();
        }
    }


    //if that was the first mesh that was added, add also a grid for the ground 
    if(m_meshes.size()==1 && !m_meshes.back()->is_empty()){

        MeshSharedPtr mesh_grid=Mesh::create();
        mesh_grid->create_grid(8, mesh->V.col(1).minCoeff(), get_scale());
        m_meshes.push_back(mesh_grid);

    }
    
}

void Scene::show(const Mesh& mesh, const std::string name){
    show(std::make_shared<Mesh>(mesh), name);
}


void Scene::add_mesh(const std::shared_ptr<Mesh> mesh, const std::string name){

    std::lock_guard<std::mutex> lock(m_mesh_mutex);  // so that accesed to the map are thread safe

    m_meshes.push_back(mesh);
    m_meshes.back()->name=name;
    if(m_meshes.back()->V.rows()!=m_meshes.back()->NV.rows()){
        m_meshes.back()->recalculate_normals();
    }

    // // //debug
    // Eigen::MatrixXd NF;
    // NF.resize(m_meshes.back()->F.rows(),3);
    // int Frows = m_meshes.back()->F.rows();
    // // #pragma omp parallel for if (Frows>10000)
    // for(int i = 0; i < Frows;i++){
    //     VLOG(1) << "normals for face " << i << " with indices " << m_meshes.back()->F.row(i);
    //     const Eigen::Matrix<double, 1, 3> v1 = m_meshes.back()->V.row(m_meshes.back()->F(i,1)) - m_meshes.back()->V.row(m_meshes.back()->F(i,0));
    //     const Eigen::Matrix<double, 1, 3> v2 = m_meshes.back()->V.row(m_meshes.back()->F(i,2)) - m_meshes.back()->V.row(m_meshes.back()->F(i,0));
    //     VLOG(1) << "normals for face " << i << " with v1 " << v1 << " and v2 " << v2;
    //     NF.row(i) = v1.cross(v2);//.normalized();
    //     // double r = N.row(i).norm();
    //     // if(r == 0)
    //     // {
    //     //     N.row(i) = Z;
    //     // }else
    //     // {
    //     //     N.row(i) /= r;
    //     // }
    // }




    //if that was the first mesh that was added, add also a grid for the ground 
    if(m_meshes.size()==1 && !m_meshes.back()->is_empty()){

        MeshSharedPtr mesh_grid=Mesh::create();
        mesh_grid->create_grid(8, mesh->V.col(1).minCoeff(), get_scale());
        m_meshes.push_back(mesh_grid);
      
    }
    
}

void Scene::clear(){
    std::lock_guard<std::mutex> lock(m_mesh_mutex);  // so that accesed to the map are thread safe
    m_meshes.clear();
}

int Scene::get_nr_meshes(){
    std::lock_guard<std::mutex> lock(m_mesh_mutex);  // so that accesed to the map are thread safe
    return m_meshes.size();
}

int Scene::get_total_nr_vertices(){
    std::lock_guard<std::mutex> lock(m_mesh_mutex);  // so that accesed to the map are thread safe
    int V_nr=0;
    for (size_t i = 0; i < m_meshes.size(); i++) {
        if(m_meshes[i]->m_vis.m_is_visible){
            V_nr+=m_meshes[i]->V.rows();
        }
    }
    return V_nr;
}

int Scene::get_total_nr_faces(){
    std::lock_guard<std::mutex> lock(m_mesh_mutex);  // so that accesed to the map are thread safe
    int F_nr=0;
    for (size_t i = 0; i < m_meshes.size(); i++) {
        if(m_meshes[i]->m_vis.m_is_visible){
            F_nr+=m_meshes[i]->F.rows();
        }
    }
    return F_nr;
}

std::shared_ptr<Mesh> Scene::get_mesh_with_name(const std::string name){
    std::lock_guard<std::mutex> lock(m_mesh_mutex);  // so that accesed to the map are thread safe
    for (size_t i = 0; i < m_meshes.size(); i++) {
        if(m_meshes[i]->name==name){
            return m_meshes[i];
        }
    }
    LOG_S(FATAL) << "No mesh with name " << name;
    return m_meshes[0]; //HACK because this line will never occur because the previous line will kill it but we just put it to shut up the compiler warning
}

std::shared_ptr<Mesh> Scene::get_mesh_with_idx(const unsigned int idx){
    std::lock_guard<std::mutex> lock(m_mesh_mutex);  // so that accesed to the map are thread safe
    if(idx<m_meshes.size()){
        return m_meshes[idx];
    }else{
        LOG_S(FATAL) << "No mesh with idx " << idx;
        return m_meshes[0]; //HACK because this line will never occur because the previous line will kill it but we just put it to shut up the compiler warning
    }
}

int Scene::get_idx_for_name(const std::string name){
    std::lock_guard<std::mutex> lock(m_mesh_mutex);  // so that accesed to the map are thread safe
    for (size_t i = 0; i < m_meshes.size(); i++) {
        if(m_meshes[i]->name==name){
            return i;
        }
    }
    LOG_S(FATAL) << "No mesh with name " << name;
    return -1; //HACK because this line will never occur because the previous line will kill it but we just put it to shut up the compiler warning
}

bool Scene::does_mesh_with_name_exist(const std::string name){
    std::lock_guard<std::mutex> lock(m_mesh_mutex);  // so that accesed to the map are thread safe
    for (size_t i = 0; i < m_meshes.size(); i++) {
        if(m_meshes[i]->name==name){
            return true;
        }
    }
    return false;
}

void Scene::remove_meshes_starting_with_name(const std::string name_prefix){
    std::lock_guard<std::mutex> lock(m_mesh_mutex);  // so that accesed to the map are thread safe

    std::vector< std::shared_ptr<Mesh> > meshes_filtered;

    for(int i=0; i<m_meshes.size(); i++){
        VLOG(1) << "checking mesh with name " << m_meshes[i]->name;
        if(! m_meshes[i]->name.find(name_prefix, 0) == 0){
            VLOG(1) << "Keeping mesh " << m_meshes[i]->name;
            //mesh does not have the prefix, we keep it
            meshes_filtered.push_back(m_meshes[i]);
        }else{
            VLOG(1) << "Removing mesh " << m_meshes[i]->name;
        }

    }

    m_meshes=meshes_filtered;

}

// void Scene::update(const Mesh& mesh, const std::string name){
//     if(does_mesh_with_name_exist(name)){
//         Mesh& to_be_updated=get_mesh_with_name(name);
//         to_be_updated=mesh;
//         to_be_updated.name=name;
//         to_be_updated.m_is_dirty=true;
//         recalculate_normals
//     }else{
//         add_mesh(mesh, name); //doesn't exist, add it to the scene
//     }
// }


Eigen::Vector3f Scene::get_centroid(){

    //if the scene is empty just return the 0.0.0 
    if(is_empty()){
        return Eigen::Vector3f::Zero();
    }


    Eigen::MatrixXd min_point_per_mesh; // each row stores the minimum point of the corresponding mesh. 
    Eigen::MatrixXd max_point_per_mesh; // each row stores the minimum point of the corresponding mesh. 
    min_point_per_mesh.resize(m_meshes.size(), 3);
    max_point_per_mesh.resize(m_meshes.size(), 3);
    min_point_per_mesh.setZero();
    max_point_per_mesh.setZero();
    for(size_t i=0; i<m_meshes.size(); i++){
        if(m_meshes[i]->is_empty()){ continue; }  
        min_point_per_mesh.row(i) = m_meshes[i]->V.colwise().minCoeff();   
        max_point_per_mesh.row(i) = m_meshes[i]->V.colwise().maxCoeff();   
    }

    //absolute minimum between all meshes
    Eigen::Vector3d min_point = min_point_per_mesh.colwise().minCoeff(); 
    Eigen::Vector3d max_point = max_point_per_mesh.colwise().maxCoeff();
    Eigen::Vector3d centroid = (0.5*(min_point + max_point)).eval();

    return centroid.cast<float>();
}
  
float Scene::get_scale(){

    //if the scene is empty just return the scale 1.0 
    if(is_empty()){
       return 1.0; 
    }


    Eigen::MatrixXd min_point_per_mesh; // each row stores the minimum point of the corresponding mesh. 
    Eigen::MatrixXd max_point_per_mesh; // each row stores the minimum point of the corresponding mesh. 
    min_point_per_mesh.resize(m_meshes.size(), 3);
    max_point_per_mesh.resize(m_meshes.size(), 3);
    min_point_per_mesh.setConstant(std::numeric_limits<float>::max());
    max_point_per_mesh.setConstant(std::numeric_limits<float>::lowest());
    for(size_t i=0; i<m_meshes.size(); i++){
        if(m_meshes[i]->is_empty()){ continue; }  
        if(m_meshes[i]->name=="grid_floor"){
            continue;
        }
        min_point_per_mesh.row(i) = m_meshes[i]->V.colwise().minCoeff();   
        max_point_per_mesh.row(i) = m_meshes[i]->V.colwise().maxCoeff();   
    }

    //absolute minimum between all meshes
    Eigen::Vector3d min_point = min_point_per_mesh.colwise().minCoeff(); 
    Eigen::Vector3d max_point = max_point_per_mesh.colwise().maxCoeff();
    Eigen::Vector3d centroid = (0.5*(min_point + max_point)).eval();

    // VLOG(1) << "min_point " << min_point.transpose();
    // VLOG(1) << "max_point " << max_point.transpose();
    float scale = (max_point-min_point).array().abs().maxCoeff();

    // VLOG(1) << "scale is " << scale;

    return scale;
}

bool Scene::is_empty(){
    //return true when all of the meshes have no vertices`
    for(size_t i=0; i<m_meshes.size(); i++){
        if(!m_meshes[i]->is_empty()){
            return false;
        }
    }
    return true;
}
