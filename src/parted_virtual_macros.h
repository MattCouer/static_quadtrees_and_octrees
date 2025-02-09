#ifndef PARTED_VIRTUAL_MACROS
#define PARTED_VIRTUAL_MACROS
#include <godot_cpp/core/gdvirtual.gen.inc>

/*
Macros to ameliorate the assumption of the gdextension implementation that we are not working with incomplete types.
Essentially, the original macros are parted into declarations and definitions. As well is __FORCE_INLINE__ removed for linking
across files.
*/


namespace godot
{

    #define GDVIRTUAL0R_DECLARE(m_ret, m_name)\
        ::godot::StringName _gdvirtual_##m_name##_sn = #m_name;\
        template <bool required>\
        bool _gdvirtual_##m_name##_call(m_ret &r_ret);\
        bool _gdvirtual_##m_name##_overridden() const;\
        static ::godot::MethodInfo _gdvirtual_##m_name##_get_method_info();

    #define GDVIRTUAL0R_DEFINE(m_class_name, m_ret, m_name)\
        template <bool required>\
        bool m_class_name::_gdvirtual_##m_name##_call(m_ret &r_ret) {\
            if (::godot::internal::gdextension_interface_object_has_script_method(_owner, &_gdvirtual_##m_name##_sn)) { \
                GDExtensionCallError ce;\
                ::godot::Variant ret;\
                ::godot::internal::gdextension_interface_object_call_script_method(_owner, &_gdvirtual_##m_name##_sn, nullptr, 0, &ret, &ce);\
                if (ce.error == GDEXTENSION_CALL_OK) {\
                    r_ret = ::godot::VariantCaster<m_ret>::cast(ret);\
                    return true;\
                }\
            }\
            if (required) {\
                ERR_PRINT_ONCE("Required virtual method " + get_class() + "::" + #m_name + " must be overridden before calling.");\
                (void)r_ret;\
            }\
            return false;\
        }\
        bool m_class_name::_gdvirtual_##m_name##_overridden() const {\
            return ::godot::internal::gdextension_interface_object_has_script_method(_owner, &_gdvirtual_##m_name##_sn); \
        }\
        ::godot::MethodInfo m_class_name::_gdvirtual_##m_name##_get_method_info() {\
            ::godot::MethodInfo method_info;\
            method_info.name = #m_name;\
            method_info.flags = ::godot::METHOD_FLAG_VIRTUAL;\
            method_info.return_val = ::godot::GetTypeInfo<m_ret>::get_class_info();\
            method_info.return_val_metadata = ::godot::GetTypeInfo<m_ret>::METADATA;\
            return method_info;\
        }

} // namespace godot

#endif