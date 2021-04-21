#ifndef DATA_MAPPER
#define DATA_MAPPER

#include <iostream>
#include <vector>
#include <algorithm>

#include "data_mapper_interface.hpp"
#include "database.hpp"

extern Database db;

template<class T>
class DataMapper : public DataMapperInterface<T> {
    using DataMapperInterface<T>::cache_unit;

    public:
        DataMapper();

        std::vector<T>& Read_all() override;
        T& Read(int) override;
        int Create(T&) override;
        void Update(T&) override;
        void Delete(int) override;
        void Delete_all() override;

        int getCacheUnitSize();
};


class DataMapperException : public std::exception {
    std::string _message;
    public:
        DataMapperException(const std::string & message):_message(message) {}
        const char * what() const throw() {
            return _message.c_str();
        }
};


template<class T>
DataMapper<T>::DataMapper(){
    std::cout << "Reading " + get_relation_name(T()) + " from DB..." << '\n';
    Read_all();               //Cache Initialization from DB
}


template<class T>
int DataMapper<T>::getCacheUnitSize() {
    return cache_unit.size();
}


template<class T>
std::vector<T>& DataMapper<T>::Read_all(){
    if (getCacheUnitSize()==0){
        cache_unit = db.select_all<T>();
    }
    return cache_unit;
}


template<class T>
T& DataMapper<T>::Read(int id) {
    std::string relation_name = get_relation_name(T());

    typename std::vector<T>::iterator it;
    it = std::find(cache_unit.begin(), cache_unit.end(), T(id));     //conversion constructor
    if(it!=cache_unit.end()){
        std::cout << relation_name + " with id: " << (*it).getId() << " found in cache!" << std::endl;
        return *it;
    }
    else {
        std::cout << relation_name + " with id: " << id << " not found in cache!" << std::endl;
        std::cout << "Searching in DB ..." << std::endl;
        try {
            cache_unit.push_back(db.select<T>(id));
            std::cout << relation_name + " found in DB. Loading in cache..." << std::endl;
            return cache_unit.back();
        }
        catch(DatabaseException& e){
            std::cout << e.what() << std::endl;
            throw DataMapperException("Read failed!");
        }
    }
}


template<class T>
int DataMapper<T>::Create(T& t) {
    std::string relation_name = get_relation_name(t);
    try {
        int id_created = db.insert(t);
        // success insert
        t.setId(id_created);
        cache_unit.push_back(t);
        std::cout << relation_name + " with id: " << t.getId() << " added!" << std::endl;
        return t.getId();
    }
    catch(DatabaseException& e){
           std::cout << e.what() << std::endl;
           throw DataMapperException("Create failed!");
    }
}


template<class T>
void DataMapper<T>::Update(T& t){
    std::string relation_name = get_relation_name(t);

    typename std::vector<T>::iterator it;
    it = std::find(cache_unit.begin(), cache_unit.end(), t);
    if(it!=cache_unit.end()){
        try {
            // update DB
            db.update(t);
            // update cache
            (*it) = t;
            std::cout << relation_name + " with id: " << (*it).getId() << " updated!" << std::endl;
        }
        catch(DatabaseException& e){
            std::cout << e.what() << std::endl;
            throw DataMapperException("Update failed!");
        }
    }
    else {
        try {
            // find in DB
            Read(t.getId());
            Update(t);
        }
        catch(DataMapperException& e){
            std::cout << "Update failed!" << std::endl;
            throw DataMapperException("Update failed!");
        }
    }
}


template<class T>
void DataMapper<T>::Delete(int id) {
    std::string relation_name = get_relation_name(T());
    try {
        //remove from db
        db.delete_f<T>(id);

        //remove from cache (Erase–remove idiom)
        cache_unit.erase(std::remove(cache_unit.begin(), cache_unit.end(), T(id)), cache_unit.end());

        std::cout << relation_name + " with id: " << id << " deleted!" << std::endl;
    }
    catch(DatabaseException& e) {
        std::cout << e.what()<< std::endl;
        throw DataMapperException("Delete failed!");
    }
    catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        throw DataMapperException("Delete failed!");
    }
}


template<class T>
void DataMapper<T>::Delete_all(){
    std::string relation_name = get_relation_name(T());
    try {
        //remove from db
        db.full_delete_f<T>();
        //remove from cache
        cache_unit.erase(cache_unit.begin(), cache_unit.end());

        std::cout << "All records in "+ relation_name + " deleted!" << std::endl;
    }
    catch(DatabaseException& e) {
        std::cout << e.what()<< std::endl;
        throw DataMapperException("Full-Delete failed!");
    }
    catch(std::exception& e) {
        std::cout << e.what() << std::endl;
        throw DataMapperException("Full-Delete failed!");
    }
}

#endif
