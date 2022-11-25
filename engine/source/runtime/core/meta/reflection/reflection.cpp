#include "core/meta/reflection/reflection.h"

#include <map>
#include <string>

namespace Piccolo
{
    namespace Reflection
    {
        const char* k_unknown_type = "UnknownType";
        const char* k_unknown      = "Unknown";

        static std::map<std::string, ClassFunctionTuple*>      m_class_map;
        static std::multimap<std::string, FieldFunctionTuple*> m_field_map;
        static std::map<std::string, ArrayFunctionTuple*>      m_array_map;

        void TypeMetaRegisterinterface::registerToClassMap(const char* name, ClassFunctionTuple* value)
        {
            if (m_class_map.find(name) == m_class_map.end())
                m_class_map.insert(std::make_pair(name, value));
            else
                delete value;
        }

        void TypeMetaRegisterinterface::registerToFieldMap(const char* name, FieldFunctionTuple* value)
        {
            m_field_map.insert(std::make_pair(name, value));
        }

        void TypeMetaRegisterinterface::registerToArrayMap(const char* name, ArrayFunctionTuple* value)
        {
            if (m_array_map.find(name) == m_array_map.end())
                m_array_map.insert(std::make_pair(name, value));
            else
                delete value;
        }

        void TypeMetaRegisterinterface::unregisterAll()
        {
            for (const auto& itr : m_field_map)
                delete itr.second;
            m_field_map.clear();

            for (const auto& itr : m_class_map)
                delete itr.second;
            m_class_map.clear();

            for (const auto& itr : m_array_map)
                delete itr.second;
            m_array_map.clear();
        }

        TypeMeta::TypeMeta() : m_type_name(k_unknown_type), m_is_valid(false) { m_fields.clear(); }

        TypeMeta::TypeMeta(std::string type_name) : m_type_name(type_name)
        {
            m_is_valid = false;
            m_fields.clear();

            for (auto fields_iter = m_field_map.equal_range(type_name); fields_iter.first != fields_iter.second;
                 fields_iter.first++)
            {
                FieldAccessor f_field(fields_iter.first->second);
                m_fields.emplace_back(
                    f_field); // 通过emplace_back来取消拷贝与析构f_field的这一步，直接原地构造后添加到vector
                m_is_valid = true;
            }
        }

        TypeMeta& TypeMeta::operator=(const TypeMeta& dest)
        {
            if (this == &dest)
                return *this;
            m_fields.clear();
            m_fields    = dest.m_fields;
            m_type_name = dest.m_type_name;
            m_is_valid  = dest.m_is_valid;
            return *this;
        }

        ReflectionInstance TypeMeta::newFromNameAndJson(std::string type_name, const Json& json_context)
        {
            auto iter = m_class_map.find(type_name);
            if (iter != m_class_map.end())
                return ReflectionInstance(TypeMeta(type_name), (std::get<1>(*iter->second)(json_context)));
            return ReflectionInstance();
        }

        Json TypeMeta::writeByName(std::string type_name, void* instance)
        {
            auto iter = m_class_map.find(type_name);
            return (iter != m_class_map.end()) ? std::get<2>(*iter->second)(instance) : Json();
        }

        bool TypeMeta::newArrayAccessorFromName(std::string array_type_name, ArrayAccessor& accessor)
        {
            auto iter = m_array_map.find(array_type_name);
            if (iter != m_array_map.end())
            {
                ArrayAccessor new_accessor(iter->second);
                accessor = new_accessor;
                return true;
            }
            return false;
        }

        int TypeMeta::getFieldsList(FieldAccessor*& out_list)
        {
            int count = m_fields.size();
            out_list  = new FieldAccessor[count];
            for (int i = 0; i < count; ++i)
                out_list[i] = m_fields[i];
            return count;
        }

        int TypeMeta::getBaseClassReflectionInstanceList(ReflectionInstance*& out_list, void* instance)
        {
            auto iter = m_class_map.find(m_type_name);
            return (iter != m_class_map.end()) ? (std::get<0>(*iter->second))(out_list, instance) : 0;
        }

        FieldAccessor TypeMeta::getFieldByName(const char* name)
        {
            const auto it = std::find_if(m_fields.begin(), m_fields.end(), [&](const auto& i) {
                return std::strcmp(i.getFieldName(), name) == 0;
            });
            return (it != m_fields.end()) ? *it : FieldAccessor(nullptr);
        }

        FieldAccessor::FieldAccessor(FieldFunctionTuple* functions) : m_functions(functions)
        {
            m_field_type_name = k_unknown_type;
            m_field_name      = k_unknown;
            if (m_functions == nullptr)
                return;

            m_field_type_name = (std::get<4>(*m_functions))();
            m_field_name      = (std::get<3>(*m_functions))();
        }

        FieldAccessor& FieldAccessor::operator=(const FieldAccessor& dest)
        {
            if (this == &dest)
                return *this;
            m_functions       = dest.m_functions;
            m_field_name      = dest.m_field_name;
            m_field_type_name = dest.m_field_type_name;
            return *this;
        }

        ArrayAccessor::ArrayAccessor(ArrayFunctionTuple* array_func) : m_func(array_func)
        {
            m_array_type_name   = k_unknown_type;
            m_element_type_name = k_unknown_type;
            if (m_func == nullptr)
                return;

            m_array_type_name   = std::get<3>(*m_func)();
            m_element_type_name = std::get<4>(*m_func)();
        }

        ArrayAccessor& ArrayAccessor::operator=(const ArrayAccessor& dest)
        {
            if (this == &dest)
                return *this;
            m_func              = dest.m_func;
            m_array_type_name   = dest.m_array_type_name;
            m_element_type_name = dest.m_element_type_name;
            return *this;
        }
    } // namespace Reflection
} // namespace Piccolo