#include "reflection.h"

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

                ++fields_iter.first;
            }
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

        TypeMeta FieldAccessor::getOwnerTypeMeta()
        {
            // todo: should check validation
            TypeMeta f_type((std::get<2>(*m_functions))());
            return f_type;
        }

        int TypeMeta::getFieldsList(FieldAccessor*& out_list)
        {
            int count = m_fields.size();
            out_list  = new FieldAccessor[count];
            for (int i = 0; i < count; ++i)
            {
                out_list[i] = m_fields[i];
            }
            return count;
        }

        FieldAccessor TypeMeta::getFieldByName(const char* name)
        {
            const auto it = std::find_if(m_fields.begin(), m_fields.end(), [&](const auto& i) {
                return std::strcmp(i.getFieldName(), name) == 0;
            });
            if (it != m_fields.end())
                return *it;
            return FieldAccessor(nullptr);
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