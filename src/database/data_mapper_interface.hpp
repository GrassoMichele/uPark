#ifndef DATA_MAPPER_INTERFACE
#define DATA_MAPPER_INTERFACE

template<class T>
class DataMapperInterface {
    protected:
        std::vector<T> cache_unit;
    public:
        virtual std::vector<T>& Read_all() =0;
        virtual T& Read(int) =0;
        virtual int Create(T&) =0;
        virtual void Update(T&) =0;
        virtual void Delete(int) =0;
        virtual void Delete_all() =0;
};

#endif
