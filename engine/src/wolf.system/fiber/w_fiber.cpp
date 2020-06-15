#include "w_fiber.hpp"
#include <unordered_map>
#include <boost/fiber/all.hpp>
#include <boost/lexical_cast.hpp>
#include <string>

typedef struct
{
    boost::fibers::fiber*    fibers;
    size_t                   number_of_fibers;
} fibers_info;

std::unordered_map<const char*, fibers_info*> s_schedulers;

template<typename w_arg>
W_RESULT w_fiber_scheduler_init(_In_z_ const char* pSchedulerName,
                                _In_ std::initializer_list<const w_arg&> pFiberTasks)
{
    fibers_info* _fibers_info = nullptr;
    size_t _i = 0;
    
    auto _size_of_tasks = pFiberTasks.size();
    if (!_size_of_tasks)
    {
        goto _return;
    }
    
    _fibers_info =  (fibers_info*)w_alloc(sizeof(fibers_info), "w_fiber_scheduler_init::_fibers_info");
    if (!_fibers_info)
    {
        goto _return;
    }
    
    _fibers_info->fibers = (boost::fibers::fiber*)w_alloc(_size_of_tasks * sizeof( boost::fibers::fiber),
                                                          "w_fiber_scheduler_init::_fibers_info->fibers");
    if (!_fibers_info->fibers)
    {
        goto _return;
    }
    
    _fibers_info->number_of_fibers = _size_of_tasks;
    
    for (auto& _task : pFiberTasks)
    {
        _fibers_info->fibers[_i++](_task);
        _fibers_info->fibers[_i++].join();        
    }
    
    s_schedulers[pSchedulerName] = _fibers_info;
    
_return:
    if (_fibers_info)
    {
        w_free(_fibers_info);
        w_free(_fibers_info->fibers);
    }
    return W_SUCCESS;
}

scheduler_info*  w_fiber_get_scheduler_info(_In_z_ const char* pSchedulerName)
{
    auto _find = s_schedulers.find(pSchedulerName);
    if (_find == s_schedulers.end())
    {
        return nullptr;
    }
    
    auto _name_len = strlen(pSchedulerName);
    auto _number_of_fibers = _find->second->number_of_fibers;
    
    auto _scheduler_info =  (scheduler_info*)w_alloc(sizeof(scheduler_info), "w_fiber_get_scheduler_info::_scheduler_info");
    if (!_scheduler_info)
    {
        return nullptr;
    }
    
    _scheduler_info->name = (char*)w_alloc(_name_len + 1, "w_fiber_get_scheduler_info::_scheduler_info->name");
    if (!_scheduler_info->name)
    {
        goto _failed;
    }
    
    _scheduler_info->number_of_fibers = _number_of_fibers;
    if (_number_of_fibers)
    {
        _scheduler_info->fiber_ids = (size_t*)w_alloc(_number_of_fibers, "w_fiber_get_scheduler_info::_scheduler_info->fiber_ids");
        if (!_scheduler_info->fiber_ids)
        {
            goto _failed;
        }
        
        size_t _id = 0;
        std::string _id_str;
        for (size_t i = 0; i < _number_of_fibers; ++i)
        {
            _id_str = boost::lexical_cast<std::string>(_find->second->fibers[i].get_id());
            sscanf(_id_str.c_str(), "%lx", &_id);
            _scheduler_info->fiber_ids[i] = _id;
        }
    }
    else
    {
        _scheduler_info->fiber_ids = nullptr;
    }
    //copy name of this scheduler
    strcpy(_scheduler_info->name, pSchedulerName);
    
    return _scheduler_info;
    
_failed:
    if(_scheduler_info)
    {
        w_free(_scheduler_info);
        w_free(_scheduler_info->name);
        w_free(_scheduler_info->fiber_ids);
    }
    return nullptr;
}

W_RESULT w_fiber_is_joinable(_In_z_ const char* pSchedulerName,
                             _In_ const size_t pFiberIndex)
{
    auto _find = s_schedulers.find(pSchedulerName);
    if (_find == s_schedulers.end()) return APR_BADARG;
    
    auto _number_of_fibers = _find->second->number_of_fibers;
    if (pFiberIndex >= _number_of_fibers)
    {
        return APR_BADARG;
    }
    
    return _find->second->fibers[pFiberIndex].joinable() ? W_SUCCESS : W_FAILURE;
}

W_RESULT w_fiber_join(_In_z_ const char* pSchedulerName,
                      _In_ const size_t pFiberIndex)
{
    auto _find = s_schedulers.find(pSchedulerName);
    if (_find == s_schedulers.end()) return APR_BADARG;
    
    auto _number_of_fibers = _find->second->number_of_fibers;
    if (pFiberIndex >= _number_of_fibers)
    {
        return APR_BADARG;
    }
    
    _find->second->fibers[pFiberIndex].join();
    
    return W_SUCCESS;
}


W_RESULT w_fiber_detach(_In_z_ const char* pSchedulerName,
                      _In_ const size_t pFiberIndex)
{
    auto _find = s_schedulers.find(pSchedulerName);
    if (_find == s_schedulers.end()) return APR_BADARG;
    
    auto _number_of_fibers = _find->second->number_of_fibers;
    if (pFiberIndex >= _number_of_fibers)
    {
        return APR_BADARG;
    }
    
    _find->second->fibers[pFiberIndex].detach();
    
    return W_SUCCESS;
}

W_RESULT w_fiber_swap(_In_z_ const char* pSchedulerName,
                      _In_ const size_t pFiberIndex,
                      _In_ const size_t pAnotherFiberIndex)
{
    auto _find = s_schedulers.find(pSchedulerName);
    if (_find == s_schedulers.end()) return APR_BADARG;
    
    auto _number_of_fibers = _find->second->number_of_fibers;
    if (pFiberIndex >= _number_of_fibers || pAnotherFiberIndex >= _number_of_fibers)
    {
        return APR_BADARG;
    }
    
    _find->second->fibers[pFiberIndex].swap(_find->second->fibers[pAnotherFiberIndex]);
    
    return W_SUCCESS;
}

W_RESULT w_fiber_swap(_In_z_ const char* pSchedulerName,
                      _In_   const size_t pFiberIndex,
                      _In_z_ const char* pAnotherSchedulerName,
                      _In_   const size_t pAnotherFiberIndex)
{
    auto _find_src = s_schedulers.find(pSchedulerName);
    if (_find_src == s_schedulers.end()) return APR_BADARG;
    
    auto _find_dst = s_schedulers.find(pAnotherSchedulerName);
    if (_find_dst == s_schedulers.end()) return APR_BADARG;
    
    auto _number_of_src_fibers = _find_src->second->number_of_fibers;
    auto _number_of_dst_fibers = _find_dst->second->number_of_fibers;
    if (pFiberIndex >= _number_of_src_fibers || pAnotherFiberIndex >= _number_of_dst_fibers)
    {
        return APR_BADARG;
    }
    
    _find_src->second->fibers[pFiberIndex].swap(_find_dst->second->fibers[pAnotherFiberIndex]);
    
    return W_SUCCESS;
}