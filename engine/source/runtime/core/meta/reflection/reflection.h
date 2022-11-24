#pragma once

#include "core/meta/json.h"
#include <functional>
#include <string>

namespace Piccolo
{
#define META(...)
#define CLASS(class_name, ...) class class_name
#define STRUCT(struct_name, ...) struct struct_name

#define REFLECTION_BODY(class_name) \
    friend class Reflection::TypeFieldReflectionOparator::Type##class_name##Operator; \
    // friend class Serializer;

#define REFLECTION_TYPE(class_name) \
    namespace Reflection \
    { \
        namespace TypeFieldReflectionOparator \
        { \
            class Type##class_name##Operator; \
        } \
    };

#define REGISTER_BASE_CLASS_TO_MAP(name, value) TypeMetaRegisterinterface::registerToClassMap(name, value);
#define REGISTER_FIELD_TO_MAP(name, value) TypeMetaRegisterinterface::registerToFieldMap(name, value);
#define REGISTER_ARRAY_TO_MAP(name, value) TypeMetaRegisterinterface::registerToArrayMap(name, value);
#define UNREGISTER_ALL TypeMetaRegisterinterface::unregisterAll();

#define PICCOLO_REFLECTION_NEW(name, ...) Reflection::ReflectionPtr(#name, new name(__VA_ARGS__));
#define PICCOLO_REFLECTION_DELETE(value) \
    if (value) \
    { \
        delete (value).operator->(); \
        (value).getPtrReference() = nullptr; \
    }
#define PICCOLO_REFLECTION_DEEP_COPY(type, dst_ptr, src_ptr) \
    *static_cast<(type*)>(dst_ptr) = *static_cast<(type*)>((src_ptr).getPtr());

    namespace Reflection
    {
        class TypeMeta;
        class FieldAccessor;
        class ArrayAccessor;
        class ReflectionInstance;
    } // namespace Reflection

    using SetFuncion     = std::function<void(void*, void*)>;
    using GetFuncion     = std::function<void*(void*)>;
    using GetNameFuncion = std::function<const char*()>;
    using GetBoolFunc    = std::function<bool()>;
    using SetArrayFunc   = std::function<void(int, void*, void*)>;
    using GetArrayFunc   = std::function<void*(int, void*)>;
    using GetSizeFunc    = std::function<int(void*)>;

    using ConstructorWithJson                    = std::function<void*(const int&)>;
    using WriteJsonByName                        = std::function<Json(void*)>;
    using GetBaseClassReflectionInstanceListFunc = std::function<int(Reflection::ReflectionInstance*&, void*)>;

    using ClassFunctionTuple = std::tuple<GetBaseClassReflectionInstanceListFunc, ConstructorWithJson, WriteJsonByName>;
    using FieldFunctionTuple =
        std::tuple<SetFuncion, GetFuncion, GetNameFuncion, GetNameFuncion, GetNameFuncion, GetBoolFunc>;
    using ArrayFunctionTuple = std::tuple<SetArrayFunc, GetArrayFunc, GetSizeFunc, GetNameFuncion, GetNameFuncion>;

    namespace Reflection
    {
        class TypeMetaRegisterinterface
        {
        public:
            static void registerToClassMap(const char* name, ClassFunctionTuple* value);
            static void registerToFieldMap(const char* name, FieldFunctionTuple* value);
            static void registerToArrayMap(const char* name, ArrayFunctionTuple* value);

            static void unregisterAll();
        };

        class TypeMeta
        {
            friend class FieldAccessor;
            friend class ArrayAccessor;
            friend class TypeMetaRegisterinterface;

        public:
            TypeMeta();

            static TypeMeta newMetaFromName(std::string type_name) { return TypeMeta(type_name); }
            // static bool               newArrayAccessorFromName(std::string array_type_name, ArrayAccessor& accessor);
            // static ReflectionInstance newFromNameAndJson(std::string type_name, const Json& json_context);
            // static Json               writeByName(std::string type_name, void* instance);

            int getFieldsList(FieldAccessor*& out_list);
            // int           getBaseClassReflectionInstanceList(ReflectionInstance*& out_list, void* instance);
            FieldAccessor getFieldByName(const char* name);
            std::string   getTypeName() { return m_type_name; };
            bool          isValid() const { return m_is_valid; }

            TypeMeta& operator=(const TypeMeta& dest);

        private:
            explicit TypeMeta(std::string type_name);

            std::vector<FieldAccessor, std::allocator<FieldAccessor>> m_fields;
            std::string                                               m_type_name;
            bool                                                      m_is_valid;
        };

        class FieldAccessor
        {
            friend class TypeMeta;

        public:
            FieldAccessor();

            void* get(void* instance) { return static_cast<void*>((std::get<1>(*m_functions))(instance)); };
            void  set(void* instance, void* value) { (std::get<0>(*m_functions))(instance, value); };

            TypeMeta getOwnerTypeMeta();

            bool        getTypeMeta(TypeMeta& field_type);
            const char* getFieldName() const { return m_field_name; }
            const char* getFieldTypeName() { return m_field_type_name; }
            bool        isArrayType() { return (std::get<5>(*m_functions))(); };

            FieldAccessor& operator=(const FieldAccessor& dest);

        private:
            explicit FieldAccessor(FieldFunctionTuple* functions);

            FieldFunctionTuple* m_functions;
            const char*         m_field_name;
            const char*         m_field_type_name;
        };

        class ArrayAccessor
        {
            friend class TypeMeta;

        public:
            ArrayAccessor() : m_func(nullptr), m_array_type_name("UnKnownType"), m_element_type_name("UnKnownType") {}
            const char* getArrayTypeName() { return m_array_type_name; }
            const char* getElementTypeName() { return m_element_type_name; }

            void set(int index, void* instance, void* element_value)
            {
                size_t count = getSize(instance);
                std::get<0> (*m_func)(index, instance, element_value);
            }
            void* get(int index, void* instance)
            {
                size_t count = getSize(instance);
                return std::get<1>(*m_func)(index, instance);
            }
            int getSize(void* instance) { return std::get<2>(*m_func)(instance); }

            ArrayAccessor& operator=(const ArrayAccessor& dest);

        private:
            explicit ArrayAccessor(ArrayFunctionTuple* array_func);

            ArrayFunctionTuple* m_func;
            const char*         m_array_type_name;
            const char*         m_element_type_name;
        };

        class ReflectionInstance
        {
        public:
            ReflectionInstance(TypeMeta meta, void* instance) : m_meta(meta), m_instance(instance) {}
            ReflectionInstance() : m_instance(nullptr) {}
            ReflectionInstance& operator=(const ReflectionInstance& dest)
            {
                if (this == &dest)
                    return *this;
                m_instance = dest.m_instance;
                m_meta     = dest.m_meta;
                return *this;
            }
            ReflectionInstance& operator=(ReflectionInstance&& dest)
            {
                if (this == &dest)
                    return *this;
                m_instance = dest.m_instance;
                m_meta     = dest.m_meta;
                return *this;
            }

            TypeMeta m_meta;
            void*    m_instance;
        };

        template<typename T>
        class ReflectionPtr
        {
            template<typename U>
            friend class ReflectionPtr;

        public:
            ReflectionPtr(std::string type_name, T* instance) : m_type_name(type_name), m_instance(instance) {}
            ReflectionPtr() : m_instance(nullptr) {}

            ReflectionPtr(const ReflectionPtr& dest) : m_type_name(dest.m_type_name), m_instance(dest.m_instance) {}

            template<typename U /*, typename = typename std::enable_if<std::is_safely_castable<T*, U*>::value>::type */>
            ReflectionPtr<T>& operator=(const ReflectionPtr<U>& dest)
            {
                if (this == static_cast<void*>(&dest))
                    return *this;
                m_type_name = dest.m_type_name;
                m_instance  = static_cast<T*>(dest.m_instance);
                return *this;
            }

            template<typename U /*, typename = typename std::enable_if<std::is_safely_castable<T*, U*>::value>::type*/>
            ReflectionPtr<T>& operator=(ReflectionPtr<U>&& dest)
            {
                if (this == static_cast<void*>(&dest))
                    return *this;
                m_type_name = dest.m_type_name;
                m_instance  = static_cast<T*>(dest.m_instance);
                return *this;
            }

            ReflectionPtr<T>& operator=(const ReflectionPtr<T>& dest)
            {
                if (this == &dest)
                    return *this;
                m_type_name = dest.m_type_name;
                m_instance  = dest.m_instance;
                return *this;
            }

            ReflectionPtr<T>& operator=(ReflectionPtr<T>&& dest)
            {
                if (this == &dest)
                    return *this;
                m_type_name = dest.m_type_name;
                m_instance  = dest.m_instance;
                return *this;
            }

            std::string getTypeName() const { return m_type_name; }
            void        setTypeName(std::string name) { m_type_name = name; }

            bool operator==(const T* ptr) const { return (m_instance == ptr); }
            bool operator!=(const T* ptr) const { return (m_instance != ptr); }
            bool operator==(const ReflectionPtr<T>& rhs_ptr) const { return (m_instance == rhs_ptr.m_instance); }
            bool operator!=(const ReflectionPtr<T>& rhs_ptr) const { return (m_instance != rhs_ptr.m_instance); }

            template<
                typename T1 /*, typename = typename std::enable_if<std::is_safely_castable<T*, T1*>::value>::type*/>
            explicit operator T1*()
            {
                return static_cast<T1*>(m_instance);
            }

            template<
                typename T1 /*, typename = typename std::enable_if<std::is_safely_castable<T*, T1*>::value>::type*/>
            explicit operator ReflectionPtr<T1>()
            {
                return ReflectionPtr<T1>(m_type_name, static_cast<T1*>(m_instance));
            }

            template<
                typename T1 /*, typename = typename std::enable_if<std::is_safely_castable<T*, T1*>::value>::type*/>
            explicit operator const T1*() const
            {
                return static_cast<T1*>(m_instance);
            }

            template<
                typename T1 /*, typename = typename std::enable_if<std::is_safely_castable<T*, T1*>::value>::type*/>
            explicit operator ReflectionPtr<T1>() const
            {
                return ReflectionPtr<T1>(m_type_name, static_cast<T1*>(m_instance));
            }

            T*       operator->() { return m_instance; }
            T*       operator->() const { return m_instance; }
            T&       operator*() { return *(m_instance); }
            T*       getPtr() { return m_instance; }
            T*       getPtr() const { return m_instance; }
            const T& operator*() const { return *(static_cast<const T*>(m_instance)); }
            T*&      getPtrReference() { return m_instance; }
            explicit operator bool() const { return (m_instance != nullptr); }

        private:
            using m_type = T;

            std::string m_type_name;
            T*          m_instance {nullptr};
        };
    } // namespace Reflection
} // namespace Piccolo
