#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/cast.h>

// From the python bindings of compressed-image
#include <wrappers/dynamic_channel.h>

namespace pybind11 
{ 
    namespace detail 
    {

        template <>
        struct type_caster<compressed_py::dynamic_channel> 
        {
            PYBIND11_TYPE_CASTER(compressed_py::dynamic_channel, _("Channel"));

            bool load(handle src, bool) 
            {
                if (!src)
                    return false;

                // Try casting the object to a shared_ptr<dynamic_channel>
                object obj = reinterpret_borrow<object>(src);
                try 
                {
                    value = *obj.cast<std::shared_ptr<compressed_py::dynamic_channel>>();
                    return true;
                } 
                catch (const error_already_set&) 
                {
                    return false;
                }
            }

            static handle cast(const compressed_py::dynamic_channel& src,
                                return_value_policy /* policy */,
                                handle /* parent */
            ) 
            {
                return py::cast(std::make_shared<compressed_py::dynamic_channel>(src));
            }
        };

    } // namespace detail

} // namespace pybind11