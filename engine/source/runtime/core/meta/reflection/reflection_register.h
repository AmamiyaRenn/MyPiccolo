#pragma once

namespace Piccolo
{
    namespace Reflection
    {
        class TypeMetaRegister
        {
        public:
            static void metaRegister();
            static void metaUnregister(); // { TypeMetaRegisterinterface::unregisterAll(); }
        };
    } // namespace Reflection
} // namespace Piccolo