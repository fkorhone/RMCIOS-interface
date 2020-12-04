/*
RMCIOS - Reactive Multipurpose Control Input Output System
Copyright (c) 2018 Frans Korhonen

RMCIOS was originally developed at Institute for Atmospheric 
and Earth System Research / Physics, Faculty of Science, 
University of Helsinki, Finland

Assistance, experience and feedback from following persons have been 
critical for development of RMCIOS: Erkki Siivola, Juha Kangasluoma, 
Lauri Ahonen, Ella Häkkinen, Pasi Aalto, Joonas Enroth, Runlong Cai, 
Markku Kulmala and Tuukka Petäjä.

This file is part of RMCIOS. This notice was encoded using utf-8.

RMCIOS is free software: you can redistribute this file and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

RMCIOS is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public Licenses
along with RMCIOS.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "RMCIOS-functions.h"

// ****************************************************************
// Channel system implementation:
// ****************************************************************
#ifndef NAN
#define NAN 0.0/0.0
#endif

// Pattern for searching functions in the system
const char *function_enum_pattern[] = {
    "help", (const char *) help_rmcios,
    "create", (const char *) create_rmcios,
    "setup", (const char *) setup_rmcios,
    "write", (const char *) write_rmcios,
    "read", (const char *) read_rmcios,

    // Legacy command: (reset is now triggered with empty write)
    "reset", (const char *) write_rmcios,
    // Legacy command: 
    // (replaced with link -channel -> write link channel to_channel)
    "link", (const char *) link_rmcios,
    // Legacy name for setup: (setup is short enough by itelf)
    "conf", (const char *) setup_rmcios
};

int function_detect (const char *name, unsigned int length)
{
    int func_i;
    int i;

    for (func_i = 0; func_i < 16; func_i += 2)
    {
        const char *function = function_enum_pattern[func_i];
        for (i = 0; i < length; i++)
        {
            if (name[i] == 0)
                break;
            if (name[i] != function[i])
                break;
            if (function[i + 1] == 0)
            {
                if (name[i + 1] == 0 || name[i + 1] == ' ' || length == i + 1)
                {
                    return (int) function_enum_pattern[func_i + 1];
                }
            }
        }
    }
    return 0;
}

int function_enum (const char *name)
{
    return function_detect (name, ~0);
}

// ***********************************************************************
// Channelsystem functions that depend only on API interface functions: 
// ***********************************************************************
int create_channel (const struct context_rmcios *context,
                    const char *namebuffer, int namelen,
                    class_rmcios class_func, void *data)
{
    struct buffer_rmcios buffers[2] = {
        {
         .data = (void *) &class_func,  // function pointer as binary
         .length = sizeof (class_func),
         .size = 0,
         .required_size = sizeof (class_func),
         .trailing_size = 0,
         }
        ,
        {
         .data = (void *) &data,        // data pointer as binary
         .length = sizeof (data),
         .size = 0,
         .required_size = sizeof (data),
         .trailing_size = 0,
         }
    };
    struct buffer_rmcios name = {
        .data = (void *) namebuffer,
        .length = namelen,
        .size = 0,
        .required_size = namelen,
        .trailing_size = 0,
    };

    int new_channel_id = 0;
    struct combo_rmcios params[2] = {
        {
         .paramtype = int_rmcios,
         .num_params = 1,
         .param.iv = &new_channel_id,
         .next = 0},
        {
         .paramtype = buffer_rmcios,
         .num_params = 1,
         .param.bv = &name,
         .next = 0}
    };

    // Add the channel & store id to param[0]:
    run_channel (context, context->create,
                 create_rmcios,
                 binary_rmcios,
                 params, 2, (const union param_rmcios) buffers);
    if (namelen > 0)
    {

        // Add name for the channel
        run_channel (context, context->name,
                     write_rmcios,
                     combo_rmcios, 0, 2, (const union param_rmcios) params);
    }
    return new_channel_id;
}

// Create a channel using channel parameters as new channel name.
int create_channel_param (const struct context_rmcios *context,
                          enum type_rmcios paramtype,
                          const union param_rmcios param,
                          int index,
                          class_rmcios channel_function, void *channel_data)
{
    // Determine the name size:
    int namelen = param_string_length (context, paramtype, param, index);
    // Allocate memory for name:
    char *name = (char *) allocate_storage (context, namelen + 1, 0);
    // Get the name:
    param_to_string (context, paramtype, param, index, namelen + 1, name);

    return create_channel (context, name, namelen, channel_function,
                           channel_data);
}

/// @brief run channel
void run_channel (const struct context_rmcios *context,
                  int id,
                  enum function_rmcios function,
                  enum type_rmcios paramtype,
                  struct combo_rmcios *returnv,
                  int num_params, const union param_rmcios param)
{
    context->run_channel (context->data,
                          context,
                          id,
                          function, paramtype, returnv, num_params, param);
}

void link_channel (const struct context_rmcios *context,
                   int channel, int to_channel)
{
    int params[2] = { channel, to_channel };
    run_channel (context, context->link, write_rmcios, int_rmcios,
                 0, 2, (const union param_rmcios) params);
}

/// @brief Link the channel to another channel, specifying the linked functions
/// Linked function can also be specified, or just set to 0 (all functions)
void link_channel_function (const struct context_rmcios *context,
                            int channel,
                            int to_channel,
                            enum function_rmcios function, int to_function)
{
    int params[4] = { channel, function, to_channel, to_function };
    run_channel (context, context->link, write_rmcios, int_rmcios,
                 0, 4, (const union param_rmcios) params);
}

// Create a channel using null-terminated string as new channel name
// String should not be temporary variable.
int create_channel_str (const struct context_rmcios *context,
                        const char *channel_name,
                        class_rmcios channel_function, void *channel_data)
{
    int len;
    for (len = 0; channel_name[len] != 0; len++);       // look length of name
    return create_channel (context, channel_name, len,
                           channel_function, channel_data);
}

// Create a channel that inherits existing channel name as basename. 
// And adds suffix string to form subchannel name.
int create_subchannel_str (const struct context_rmcios *context,
                           int channel,
                           const char *suffix_str,
                           class_rmcios channel_function, void *channel_data)
{
    //info(status("create subchannel \r\n") ;
    int namelen;
    int suffixlen;
    if (suffix_str == 0)
    {
        return 0;       // Cannot create subchannel without suffix.
    }
    // Get length of suffix part
    for (suffixlen = 0; suffix_str[suffixlen] != 0; suffixlen++);
    // Get length of channel name
    namelen = channel_name (context, channel, 0, 0);
    {
        int i;
        char *subchannel_name =
            (char *) allocate_storage (context, namelen + suffixlen + 1, 0);
        if (subchannel_name == 0)
        {
            return 0;
        }
        // Get orginal channel name
        channel_name (context, channel, subchannel_name, namelen + 1);
        char *pname;
        pname = subchannel_name + namelen;
        for (i = 0; i < suffixlen; i++)
            // Append the suffix
            pname[i] = suffix_str[i];
        // insert null terminator ;
        pname[i] = 0;
        // create the new channel
        return create_channel_str (context, subchannel_name,
                                   channel_function, channel_data);
    }
}

void return_int (const struct context_rmcios *context,
                 struct combo_rmcios *returnv, int value)
{
    if (returnv == 0 || returnv->num_params == 0)
    {
        return;
    }
    run_channel (context, context->convert, write_rmcios, int_rmcios, returnv,
                 1, (const union param_rmcios) (&value));
}

void return_float (const struct context_rmcios *context,
                   struct combo_rmcios *returnv, float value)
{
    if (returnv == 0 || returnv->num_params == 0)
    {
        return;
    }
    run_channel (context, context->convert, write_rmcios, float_rmcios,
                 returnv, 1, (const union param_rmcios) (&value));
}

void return_string (const struct context_rmcios *context,
                    struct combo_rmcios *returnv, const char *string)
{
    int length;
    for (length = 0; string[length] != 0; length++);

    struct buffer_rmcios value = {
        .data = (char *) string,        // Treated as const when size = 0
        .length = length,
        .size = 0,
        .required_size = length,
        .trailing_size = 1
    };
    if (returnv == 0 || returnv->num_params == 0)
    {
        return;
    }
    run_channel (context, context->convert, write_rmcios, buffer_rmcios,
                 returnv, 1, (const union param_rmcios) (&value));
}

void return_buffer (const struct context_rmcios *context,
                    struct combo_rmcios *returnv,
                    const char *buffer, unsigned int length)
{
    struct buffer_rmcios value = {
        .data = (char *) buffer,        // Treated as const when size = 0
        .length = length,
        .size = 0,
        .required_size = length,
        .trailing_size = 0
    };

    if (returnv == 0 || returnv->num_params == 0)
    {
        return;
    }
    run_channel (context, context->convert, write_rmcios, buffer_rmcios,
                 returnv, 1, (const union param_rmcios) (&value));
}

void return_binary (const struct context_rmcios *context,
                    struct combo_rmcios *returnv,
                    const char *buffer, unsigned int length)
{
    struct buffer_rmcios value = {
        .data = (char *) buffer,        // Treated as const when size = 0
        .length = length,
        .size = 0,
        .required_size = length,
        .trailing_size = 0
    };

    if (returnv == 0 || returnv->num_params == 0)
    {
        return;
    }
    run_channel (context, context->convert, write_rmcios, binary_rmcios,
                 returnv, 1, (const union param_rmcios) (&value));
}

void return_void (const struct context_rmcios *context,
                  struct combo_rmcios *returnv)
{
    if (returnv == 0)
    {
        return;
    }

    if (returnv->paramtype == channel_rmcios)
    {
        run_channel (context, returnv->param.channel, write_rmcios,
                     buffer_rmcios, 0, 0, (const union param_rmcios) 0);
    }
    else if (returnv->paramtype == combo_rmcios)
    {
        struct combo_rmcios *creturn = returnv->param.cv;
        return_void (context, creturn);
    }
}

float param_to_float (const struct context_rmcios *context,
                      enum type_rmcios paramtype,
                      const union param_rmcios params, int index)
{
    float retfloat = 0.0 / 0.0; // NAN
    if (params.p == 0)
    {
        return retfloat;
    }
    struct combo_rmcios returnv = {
        .paramtype = float_rmcios,
        .num_params = 1,
        .param = {&retfloat},
        .next = 0
    };

    run_channel (context, context->convert, read_rmcios, paramtype, &returnv,
                 index + 1, params);
    return retfloat;
}

int param_to_integer (const struct context_rmcios *context,
                      enum type_rmcios paramtype,
                      const union param_rmcios params, int index)
{
    int retint = 0;
    if (params.cp == 0)
    {
        return retint;
    }
    struct combo_rmcios returnv = {
        .paramtype = int_rmcios,
        .num_params = 1,
        .param = {&retint},
        .next = 0
    };
    run_channel (context, context->convert, read_rmcios, paramtype, &returnv,
                 index + 1, params);
    return retint;
}

int param_to_channel (const struct context_rmcios *context,
                      enum type_rmcios paramtype,
                      const union param_rmcios params, int index)
{
    int ireturn = 0;
    struct combo_rmcios returnv = {
        .paramtype = int_rmcios,
        .num_params = 1,
        .param.iv = &ireturn
    };

    run_channel (context, context->id, read_rmcios, paramtype, &returnv,
                 index + 1, params);
    return ireturn;
}

int param_to_int (const struct context_rmcios *context,
                  enum type_rmcios paramtype,
                  const union param_rmcios params, int index)
{
    int ireturn = 0;
    struct combo_rmcios returnv = {
        .paramtype = int_rmcios,
        .num_params = 1,
        .param.iv = &ireturn
    };

    run_channel (context, context->id, read_rmcios, paramtype, &returnv,
                 index + 1, params);

    if (ireturn == 0)
    {
        run_channel (context, context->convert, read_rmcios, paramtype,
                     &returnv, index + 1, params);
    }
    return ireturn;
}

// Copy buffer and make escape character conversions. 
// Return the length of data at destination.
int copy_mem_safe (char *src, int src_len, char *dst, int dst_len)
{
    int i;
    if (dst_len < src_len)
    {
        for (i = 0; i < dst_len; i++)
        {
            dst[i] = src[i];
        }
        return dst_len;
    }
    else
    {
        for (i = 0; i < src_len; i++)
        {
            dst[i] = src[i];
        }
        return src_len;
    }
}

const char *param_to_string (const struct context_rmcios *context,
                             enum type_rmcios paramtype,
                             const union param_rmcios params,
                             int index, int maxlen, char *to_str)
{
    const char *sreturn = "";
    struct buffer_rmcios breturn = {
        .data = to_str,
        .length = 0,
        .size = maxlen,
        .required_size = 0,
        .trailing_size = 0
    };

    struct combo_rmcios copy_to = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param = {&breturn}
    };

    struct buffer_rmcios existing_buffer = { 0 };
    struct combo_rmcios fetch_to = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param = {&existing_buffer}
    };

    if (maxlen > 0)
    {
        // Copy data to user buffer:
        run_channel (context, context->convert, write_rmcios, paramtype,
                     &copy_to, index + 1, params);

        // Ensure trailing null:
        if (breturn.length >= breturn.size)
        {
            breturn.data[breturn.size - 1] = 0;
            breturn.length = breturn.size - 1;
        }
        else
        {
            breturn.data[breturn.length] = 0;
        }
        sreturn = breturn.data;
    }

    // Check if parameter is already string compatible buffer.
    // context.convert read command fills the given structure with original buffer data (if exists)
    run_channel (context, context->convert, read_rmcios, paramtype, &fetch_to,
                 index + 1, params);
    if (existing_buffer.data != 0 && existing_buffer.trailing_size > 0
        && existing_buffer.data[existing_buffer.length] == 0)
    {
        sreturn = existing_buffer.data;
    }
    return sreturn;
}

struct buffer_rmcios param_to_buffer (const struct context_rmcios *context,
                                      enum type_rmcios paramtype,
                                      const union param_rmcios params,
                                      int index, int maxlen, char *buffer)
{
    struct buffer_rmcios breturn = {
        .data = buffer,
        .length = 0,
        .size = maxlen,
        .required_size = 0,
        .trailing_size = 0
    };
    struct combo_rmcios copy_to = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param = {&breturn}
    };

    struct buffer_rmcios existing_buffer = { 0 };
    struct combo_rmcios fetch_to = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param = {&existing_buffer}
    };

    if (maxlen > 0)
    {
        // Copy data to user buffer:
        run_channel (context, context->convert, write_rmcios, paramtype,
                     &copy_to, index + 1, params);
    }

    // Check if parameter is already buffer.
    // context.convert read command fills the given structure with original buffer data (if exists)
    run_channel (context, context->convert, read_rmcios, paramtype, &fetch_to,
                 index + 1, params);

    if (existing_buffer.data != 0)
    {
        return existing_buffer;
    }
    else
    {
        return breturn;
    }
}

struct buffer_rmcios param_to_binary (const struct context_rmcios *context,
                                      enum type_rmcios paramtype,
                                      const union param_rmcios params,
                                      int index, int maxlen, void *buffer)
{
    struct buffer_rmcios breturn = {
        .data = buffer,
        .length = 0,
        .size = maxlen,
        .required_size = 0,
        .trailing_size = 0
    };
    struct combo_rmcios copy_to = {
        .paramtype = binary_rmcios,
        .num_params = 1,
        .param = {&breturn}
    };

    struct buffer_rmcios existing_buffer = { 0 };
    struct combo_rmcios fetch_to = {
        .paramtype = binary_rmcios,
        .num_params = 1,
        .param = {&existing_buffer}
    };

    if (maxlen > 0)
    {
        // Copy data to user buffer:
        run_channel (context, context->convert, write_rmcios, paramtype,
                     &copy_to, index + 1, params);
    }

    // Check if parameter is already buffer.
    // context.convert read command fills the given structure with original buffer data (if exists)
    run_channel (context, context->convert, read_rmcios, paramtype, &fetch_to,
                 index + 1, params);

    if (existing_buffer.data != 0)
    {
        return existing_buffer;
    }
    else
    {
        return breturn;
    }
}

int param_to_function (const struct context_rmcios *context,
                       enum type_rmcios paramtype,
                       const union param_rmcios param, int index)
{
    enum function_rmcios function = param_to_integer (context,
                                                      paramtype,
                                                      (const union
                                                       param_rmcios) param,
                                                      index);
    // Convert text to function indentifier
    if (function == 0)
    {
        int blen = param_buffer_alloc_size (context, paramtype, param, index);
        {
            char buffer[blen];
            struct buffer_rmcios fname = { 0 };
            fname = param_to_buffer (context, paramtype,
                                     param, index, blen, buffer);

            function = function_detect (fname.data, fname.length);
        }
    }
    return function;
}

int param_string_length (const struct context_rmcios *context,
                         enum type_rmcios paramtype,
                         const union param_rmcios param, int index)
{
    struct buffer_rmcios existing_buffer = { 0 };
    struct combo_rmcios returnv = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param = {&existing_buffer}
    };
    // Get required length for the parameter
    // context.convert read command fills the given structure with required size parameter
    run_channel (context, context->convert, read_rmcios, paramtype, &returnv,
                 index + 1, param);
    return existing_buffer.required_size + 1;
}

int param_buffer_length (const struct context_rmcios *context,
                         enum type_rmcios paramtype,
                         const union param_rmcios param, int index)
{
    struct buffer_rmcios existing_buffer = { 0 };
    struct combo_rmcios returnv = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param = {&existing_buffer}
    };
    // Get required length for the parameter
    // context.convert read command fills the given structure with required size parameter
    run_channel (context, context->convert, read_rmcios, paramtype, &returnv,
                 index + 1, param);
    return existing_buffer.required_size;
}

int param_binary_length (const struct context_rmcios *context,
                         enum type_rmcios paramtype,
                         const union param_rmcios param, int index)
{
    struct buffer_rmcios existing_buffer = { 0 };
    struct combo_rmcios returnv = {
        .paramtype = binary_rmcios,
        .num_params = 1,
        .param = {&existing_buffer}
    };
    // Get required length for the parameter
    // context.convert read command fills the given structure with required size parameter
    run_channel (context, context->convert, read_rmcios, paramtype, &returnv,
                 index + 1, param);
    return existing_buffer.required_size;
}

int param_string_alloc_size (const struct context_rmcios *context,
                             enum type_rmcios paramtype,
                             const union param_rmcios param, int index)
{
    int needed_size = 0;
    struct buffer_rmcios existing_buffer = { 0 };
    struct combo_rmcios returnv = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param = {&existing_buffer}
    };
    // Get required length for the parameter
    // context.convert read command fills the given structure with required size parameter
    run_channel (context, context->convert, read_rmcios, paramtype, &returnv,
                 index + 1, param);
    if (existing_buffer.data != 0 && existing_buffer.trailing_size > 0
        && existing_buffer.data[existing_buffer.length] == 0)
    {
        needed_size = 0;
    }
    else
    {
        needed_size = existing_buffer.required_size + 1;
    }
    return needed_size;
}

int param_buffer_alloc_size (const struct context_rmcios *context,
                             enum type_rmcios paramtype,
                             const union param_rmcios param, int index)
{
    int needed_size = 0;
    struct buffer_rmcios existing_buffer = { 0 };
    struct combo_rmcios returnv = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param = {&existing_buffer}
    };
    // Get required length for the parameter
    // context.convert read command fills the given structure with required size parameter
    run_channel (context, context->convert, read_rmcios, paramtype, &returnv,
                 index + 1, param);
    if (existing_buffer.data != 0
        && existing_buffer.required_size == existing_buffer.length)
    {
        needed_size = 0;
    }
    else
    {
        needed_size = existing_buffer.required_size;
    }
    return needed_size;
}

float read_f (const struct context_rmcios *context, int channel)
{
    float rvalue = 0;
    struct combo_rmcios returnv = {
        .paramtype = float_rmcios,
        .num_params = 1,
        .param.fv = &rvalue
    };
    run_channel (context, channel,
                 read_rmcios, float_rmcios,
                 &returnv, 0, (const union param_rmcios) 0);
    return rvalue;
}

int read_i (const struct context_rmcios *context, int channel)
{
    int rvalue = 0;
    struct combo_rmcios returnv = {
        .paramtype = int_rmcios,
        .num_params = 1,
        .param.iv = &rvalue
    };
    run_channel (context, channel,
                 read_rmcios, int_rmcios,
                 &returnv, 0, (const union param_rmcios) 0);
    return rvalue;
}

int read_str (const struct context_rmcios *context,
              int channel, char *string, int maxlen)
{
    struct buffer_rmcios sreturn = { 0 };
    struct combo_rmcios returnv = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param.bv = &sreturn
    };

    if (maxlen != 0)
        sreturn.size = maxlen - 1;
    else
        sreturn.size = 0;
    sreturn.data = string;
    sreturn.length = 0;
    sreturn.required_size = 0;
    run_channel (context, channel,
                 read_rmcios, buffer_rmcios,
                 &returnv, 0, (const union param_rmcios) 0);
    if (sreturn.size != 0)
        sreturn.data[sreturn.length] = 0;       // Add NULL-termination
    return sreturn.required_size;
}

float write_f (const struct context_rmcios *context, int channel, float value)
{
    float rvalue = 0;
    struct combo_rmcios returnv = {
        .paramtype = float_rmcios,
        .num_params = 1,
        .param.fv = &rvalue
    };
    run_channel (context, channel,
                 write_rmcios, float_rmcios,
                 &returnv, 1, (const union param_rmcios) &value);
    return rvalue;
}

float write_fv (const struct context_rmcios *context, int channel, int params,
                float *values)
{
    float rvalue = 0;
    struct combo_rmcios returnv = {
        .paramtype = float_rmcios,
        .num_params = 1,
        .param.fv = &rvalue
    };
    run_channel (context, channel,
                 write_rmcios, float_rmcios,
                 &returnv, params, (const union param_rmcios) values);
    return rvalue;
}

int write_iv (const struct context_rmcios *context, int channel, int params,
              int *values)
{
    int rvalue = 0;
    struct combo_rmcios returnv = {
        .paramtype = int_rmcios,
        .num_params = 1,
        .param.iv = &rvalue
    };
    run_channel (context, channel,
                 write_rmcios, int_rmcios,
                 &returnv, params, (const union param_rmcios) values);
    return rvalue;
}

int write_i (const struct context_rmcios *context, int channel, int value)
{
    int rvalue = 0;
    struct combo_rmcios returnv = {
        .paramtype = int_rmcios,
        .num_params = 1,
        .param.iv = &rvalue
    };
    run_channel (context, channel, write_rmcios, int_rmcios,
                 &returnv, 1, (const union param_rmcios) &value);
    return rvalue;
}

void write_str (const struct context_rmcios *context,
                int channel, const char *str, int channel_id)
{
    struct buffer_rmcios param = { 0 };
    struct combo_rmcios returnv = {
        .paramtype = channel_rmcios,
        .num_params = 1,
        .param.channel = channel_id
    };
    int i;
    // get length of string
    for (i = 0; str[i] != 0; i++);
    param.data = (char *) str;
    param.length = i;
    param.size = 0;
    param.required_size = i;
    param.trailing_size = 1;    // Trailing 0

    run_channel (context, channel,
                 write_rmcios, buffer_rmcios,
                 &returnv, 1, (const union param_rmcios) &param);
}

void write_buffer (const struct context_rmcios *context, int channel,
                   const char *buffer, int length, int channel_id)
{
    struct buffer_rmcios param = {
        .data = (char *) buffer,
        .length = length,
        .size = 0,
        .required_size = length,
        .trailing_size = 0
    };
    struct combo_rmcios returnv = {
        .paramtype = channel_rmcios,
        .num_params = 1,
        .param.channel = channel_id,
    };
    run_channel (context, channel,
                 write_rmcios, buffer_rmcios,
                 &returnv, 1, (const union param_rmcios) &param);
}

int write_binary (const struct context_rmcios *context,
                  int channel,
                  const char *buffer, int length, char *rbuffer, int maxlen)
{
    struct buffer_rmcios param = {
        .data = (char *) buffer,
        .length = length,
        .size = 0,
        .required_size = length,
        .trailing_size = 0
    };
    struct buffer_rmcios breturnv = {
        .data = rbuffer,
        .length = 0,
        .size = maxlen,
        .required_size = 0,
        .trailing_size = 0,
    };
    struct combo_rmcios returnv = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param.bv = &breturnv
    };
    run_channel (context, channel,
                 write_rmcios, binary_rmcios,
                 &returnv, 1, (const union param_rmcios) &param);

    return breturnv.required_size;
}

int linked_channels (const struct context_rmcios *context, int channel)
{
    int ireturn = 0;
    struct combo_rmcios returnv = {
        .paramtype = int_rmcios,
        .num_params = 1,
        .param.iv = &ireturn
    };
    run_channel (context, context->link,
                 read_rmcios, int_rmcios,
                 &returnv, 1, (const union param_rmcios) &channel);
    return ireturn;
}

void *allocate_storage (const struct context_rmcios *context, int size,
                        int storage_channel)
{

    struct buffer_rmcios breturnv = { 0 };
    struct combo_rmcios returnv = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param.bv = &breturnv
    };
    struct buffer_rmcios param = {
        .data = (void *) &size,
        .length = sizeof (size),
        .size = 0,
        .required_size = sizeof (size),
        .trailing_size = 0
    };
    void *ptr = 0;
    breturnv.data = (void *) &ptr;
    breturnv.length = 0;
    breturnv.size = sizeof (ptr);

    if (storage_channel == 0)
    {
        storage_channel = context->mem;
    }
    //return (void *) write_i(storage_channel, size) ;
    run_channel (context, storage_channel,
                 write_rmcios, binary_rmcios,
                 &returnv, 1, (const union param_rmcios) &param);
    return ptr;
}

void free_storage (const struct context_rmcios *context, void *ptr,
                   int storage_channel)
{
    struct buffer_rmcios param[2] = { 0 };
    param[1].data = (char *) &ptr;
    param[1].length = sizeof (ptr);
    param[1].required_size = sizeof (ptr);
    if (storage_channel == 0)
        storage_channel = context->mem;

    run_channel (context, storage_channel, write_rmcios,
                 binary_rmcios, 0, 2, (const union param_rmcios) param);
}

int channel_enum (const struct context_rmcios *context,
                  const char *channel_name)
{
    int ireturn = 0;
    struct buffer_rmcios param;
    struct combo_rmcios returnv = {
        .paramtype = int_rmcios,
        .num_params = 1,
        .param.iv = &ireturn
    };
    int slen;
    param.data = (void *) channel_name;
    // strlen(channel_name) :
    for (slen = 0; channel_name[slen] != 0; slen++);
    param.length = slen;
    param.required_size = slen;
    param.size = 0;
    param.trailing_size = 0;
    run_channel (context, context->id,
                 read_rmcios, buffer_rmcios,
                 &returnv, 1, (const union param_rmcios) &param);
    return ireturn;
}

int channel_name (const struct context_rmcios *context,
                  int channel_id, char *name_to, int maxlen)
{
    struct buffer_rmcios breturnv = {
        .data = name_to,
        .length = 0,
        .size = maxlen,
        .required_size = 0,
        .trailing_size = 0
    };
    struct combo_rmcios returnv = {
        .paramtype = buffer_rmcios,
        .num_params = 1,
        .param.bv = &breturnv
    };
    run_channel (context, context->name,
                 read_rmcios, int_rmcios,
                 &returnv, 1, (const union param_rmcios) &channel_id);
    return breturnv.required_size;
}

// Creation of buffer structures:
struct buffer_rmcios make_str_as_const_buffer (const char *str)
{
    struct buffer_rmcios pb;
    int i;
    // get length of string
    for (i = 0; str[i] != 0; i++);
    pb.data = (char *) str;
    pb.length = i;
    pb.size = 0;
    pb.required_size = i;
    pb.trailing_size = 1;
    return pb;
}

struct buffer_rmcios make_const_buffer (const char *buffer, int length)
{
    struct buffer_rmcios pb = {
        .data = (char *) buffer,
        .length = length,
        .size = 0,
        .required_size = length,
        .trailing_size = 0
    };
    return pb;
}

// Creation of buffer structures:
struct buffer_rmcios make_str_as_buffer (char *str, int size)
{
    struct buffer_rmcios pb;
    int i;
    // get length of string
    for (i = 0; str[i] != 0; i++);
    pb.data = (char *) str;
    pb.length = i;
    pb.size = size;
    pb.required_size = i;
    pb.trailing_size = 0;
    return pb;
}

struct buffer_rmcios make_buffer (char *buffer, int length)
{
    struct buffer_rmcios pb;
    pb.data = (char *) buffer;
    pb.length = length;
    pb.size = length;
    pb.required_size = length;
    pb.trailing_size = 0;
    return pb;
}
