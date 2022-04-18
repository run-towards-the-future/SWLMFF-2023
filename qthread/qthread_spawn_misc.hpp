#include <tuple>
#ifdef __sw_slave__
#include <utility>
template<typename T, typename ...Args, int ...Is>
void qthread_obj_apply(std::tuple<T*, void (T::*)(Args...), Args...> *args_tuple, std::integer_sequence<int, Is...> seq){
  T *obj = std::get<0>(*args_tuple);
  void (T::*mem)(Args...) = (void (T::*)(Args...))std::get<1>(*args_tuple);
  (obj->*mem)(std::get<Is+2>(*args_tuple)...);
}
template<typename T, typename ...Args>
void qthread_spawn_obj_proxy(std::tuple<T*, void (T::*)(Args...), Args...> *args_tuple){
  qthread_obj_apply(args_tuple, std::make_integer_sequence<int, sizeof...(Args)>{});
}
template<typename T, typename ...Args>
auto can_spawn(void (T::*mem)(Args...)){
  return (void (*)(std::tuple<T*, void (T::*)(Args...), Args...>*))qthread_spawn_obj_proxy;
}
template<typename ...Args, int ...Is>
void qthread_va_apply(std::tuple<void (*)(Args...), Args...> *args_tuple, std::integer_sequence<int, Is...> seq){
  void (*func)(Args...) = std::get<0>(*args_tuple);
  func(std::get<Is+1>(*args_tuple)...);
}
template<typename ...Args>
void qthread_spawn_va_proxy(std::tuple<void (*)(Args...), Args...> *args_tuple){  
  qthread_va_apply(args_tuple, std::make_integer_sequence<int, sizeof...(Args)>{});
}
template void qthread_spawn_va_proxy(std::tuple<void (*)(int, int), int, int> *);
template<typename ...Args>
auto can_spawn(void (*func)(Args...)) {
  return (void(*)(std::tuple<void (*)(Args...), Args...>*))qthread_spawn_va_proxy;
}
#define CAN_SPAWN(x) template auto can_spawn(decltype(&x));
#endif
#ifdef __sw_host__
template<typename T, typename ...Args>
void slave_qthread_spawn_obj_proxy(std::tuple<T*, void (T::*)(Args...), Args...> *args_tuple);
template<typename T, typename TS, typename ...Args>
void qthread_spawn_obj(T *obj, void (TS::*mem)(Args...), Args... args){
  auto args_tuple = std::make_tuple(obj, (void (T::*)(Args...))mem, args...);
  qthread_spawn((void (*)(decltype(args_tuple)*))slave_qthread_spawn_obj_proxy, &args_tuple);
}

template<typename ...Args>
void slave_qthread_spawn_va_proxy(std::tuple<void (*)(Args...), Args...> *);
template<typename ...Args>
void qthread_spawn_va(void (*func)(Args...), Args... args){
  auto args_tuple = std::make_tuple(func, args...);
  qthread_spawn(slave_qthread_spawn_va_proxy, &args_tuple);
}
#endif
