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

/**
 * @file RMCIOS-functions.h
 * @author Frans Korhonen
 * @brief File contains channel handling functions that use channel API.
 *
 * Changelog: (date,who,description)
 *
 * */
#ifndef channel_functions_h
#define channel_functions_h

#ifdef __cplusplus
extern "C"
{
#endif

#include "RMCIOS-api.h"
#include "shared_resource.h"

#define info(context, channel,message,...) if(channel!=0) write_str(context, channel, message, 0)

// Macro for adding flags for dynamic linkage flags
#ifndef API_ENTRY_FUNC
#define API_ENTRY_FUNC
#else
#define INDEPENDENT_CHANNEL_MODULE
#endif

// ***********************************************************************
// Channelsystem functions that depend mostly only on API interface functions: 
// ***********************************************************************

/// @brief Create a channel using channel parameters as new channel name.
/// 
/// Helper function for implementing channels 
/// @param context pointer to target system context
/// @param paramtype type of parameters given in array @p param
/// @param param array of parameters
/// @param index index of parameter that contains the new channel name.
/// @param channel_function pointer to function that implement the channel
/// @param channel_data pointer to allocated channel member data 
/// @return Handle to the created channel. 0 on failure.
///
/// @snippet examples.c create_channel_param
    int create_channel_param (const struct context_rmcios *context,
                              enum type_rmcios paramtype,
                              const union param_rmcios param,
                              int index,
                              class_rmcios channel_function,
                              void *channel_data);

/// @brief Create a channel using string.
/// 
/// Channel name is given as null-terminated string.
/// @param context pointer to target system context
/// @param channel_name name for channel to create
/// @param channel_function pointer to function that implement the channel
/// @param channel_data pointer to allocated channel member data 
/// @return Handle to the created channel. 0 on failure.
///
/// @snippet examples.c create_channel_str
    int create_channel_str (const struct context_rmcios *context,
                            const char *channel_name,
                            class_rmcios channel_function,
                            void *channel_data);


/// @brief Create a channel that inherits existing channel name as basename.
/// 
/// Adds suffix string to form subchannel name.
/// @param context pointer to target system context
/// @param channel handle of channel to inherit basename from
/// @param suffix suffix to be added to the basename
/// @param channel_function pointer to function that implement the channel
/// @param channel_data pointer to allocated channel member data 
///
/// @snippet examples.c create_subchannel_str
    int create_subchannel_str (const struct context_rmcios *context,
                               int channel,
                               const char *suffix,
                               class_rmcios channel_function,
                               void *channel_data);

/// @brief Simplified link function for linking just the channel
/// 
/// @param context pointer to target system context
/// @param channel handle of source channel
/// @param to_channel handle of destination channel
///
/// @snippet examples.c link_channel
    void link_channel (const struct context_rmcios *context,
                       int channel, int to_channel);

    void link_channel_function (const struct context_rmcios *context,
                                int channel,
                                int to_channel,
                                enum function_rmcios function,
                                int to_function);

/// @brief Return a single integer from a channel:
/// 
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of returnv parameter
/// @param returnv pointer to return parameter
/// @param value integer value to be returned
///
/// @snippet examples.c return_int
    void return_int (const struct context_rmcios *context,
                     enum type_rmcios paramtype,
                     union param_rmcios returnv, int value);

/// @brief Return a single float from a channel:
/// 
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of returnv parameter
/// @param returnv pointer to return parameter
/// @param value float value to be returned
///
/// @snippet examples.c return_float
    void return_float (const struct context_rmcios *context,
                       enum type_rmcios paramtype,
                       union param_rmcios returnv, float value);

/// @brief Return string from a channel:
/// 
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of returnv parameter
/// @param returnv pointer to return parameter
/// @param value NULL- terminated string to be returned
///
/// @snippet examples.c return_string
    void return_string (const struct context_rmcios *context,
                        enum type_rmcios paramtype,
                        union param_rmcios returnv, const char *string);

/// @brief Return buffer from a channel:
/// 
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of returnv parameter
/// @param returnv pointer to return parameter
/// @param buffer pointer to buffer data to returned from
/// @param length bytes in return buffer
///
/// @snippet examples.c return_buffer
    void return_buffer (const struct context_rmcios *context,
                        enum type_rmcios paramtype,
                        union param_rmcios returnv,
                        const char *buffer, unsigned int length);

/// @brief Return binary data from a channel:
/// 
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of returnv parameter
/// @param returnv pointer to return parameter
/// @param buffer pointer to buffer data to returned from
/// @param length bytes in return buffer
///
/// @snippet examples.c return_binary
    void return_binary (const struct context_rmcios *context,
                        enum type_rmcios paramtype,
                        union param_rmcios returnv,
                        const char *buffer, unsigned int length);

/// @brief Return void (empty write to channel)
///
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of returnv parameter
/// @param returnv pointer to return parameter
///
/// @snippet examples.c return_void
    void return_void (const struct context_rmcios *context,
                      enum type_rmcios paramtype, union param_rmcios returnv);


    /// @brief Convert parameter to integer
/// 
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of @p param array
/// @param param array of parameters
/// @param index of the parameter to be read as integer
///
/// @snippet examples.c param_to_integer
    int param_to_integer (const struct context_rmcios *context,
                          enum type_rmcios paramtype,
                          const union param_rmcios param, int index);

/// @brief Convert parameter to float
/// 
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of @p param array
/// @param param array of parameters
/// @param index of the parameter to be read as float
/// @return float representation of the parameter
///
/// @snippet examples.c param_to_float
    float param_to_float (const struct context_rmcios *context,
                          enum type_rmcios paramtype,
                          const union param_rmcios param, int index);

/// @brief Get/convert parameter to NULL-terminated string. 
/// 
/// Helper function for implementing channels
/// Fills user given buffer with parameter string up to maxlen bytes.
/// Return pointer to orginal data on already correct string format.
/// Otherwise function returns pointer to user given buffer.
/// @param context pointer to target system context
/// @param paramtype type of @p param array
/// @param param array of parameters
/// @param index array item to be read
/// @param maxlen size of @p buffer
/// @param buffer buffer to be filled with string representation.
/// @return pointer to NULL-terminated character string.
///
/// @snippet examples.c param_to_string
    const char *param_to_string (const struct context_rmcios *context,
                                 enum type_rmcios paramtype,
                                 const union param_rmcios param,
                                 int index, int maxlen, char *buffer);

/// @brief Get/convert channel parameter to ASCII buffer. 
/// 
/// Helper function for implementing channels
/// Fills user buffer with parameter data up to maxlen bytes.
/// Returns buffer_rmcios structure pointing to orginal data 
/// Otherwise function returns structure pointing to user given buffer.
/// @param context pointer to target system context
/// @param paramtype type of @p param array
/// @param param array of parameters
/// @param index array item to be read
/// @param maxlen size of @p buffer
/// @param buffer buffer to be filled with data.
/// @return structure that contain pointer to buffer representation of the data.
///
/// @snippet examples.c return_string
    struct buffer_rmcios param_to_buffer (const struct context_rmcios
                                          *context,
                                          enum type_rmcios paramtype,
                                          const union param_rmcios param,
                                          int index, int maxlen,
                                          char *buffer);

/// @brief Get/convert channel parameter to binary array. 
/// 
/// Helper function for implementing channels
/// Fills user buffer with parameter data up to maxlen bytes.
/// Returns buffer_rmcios structure pointing to orginal on correct binary
/// Otherwise function returns structure pointing to user given buffer.
/// @param context pointer to target system context
/// @param paramtype type of @p param array
/// @param param array of parameters
/// @param index array item to be read
/// @param maxlen size of @p buffer
/// @param buffer buffer to be filled with data.
/// @return structure that contain pointer to array representation of the data.
/// Also modifiable size of the buffer is returned.
///
/// @snippet examples.c return_string
    struct buffer_rmcios param_to_binary (const struct context_rmcios
                                          *context,
                                          enum type_rmcios paramtype,
                                          const union param_rmcios param,
                                          int index, int maxlen,
                                          char *buffer);

/// @brief Convert parameter to channel identifier
/// 
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of @p param array
/// @param param array of parameters
/// @param index of the parameter to be converted as identifier
/// @return indentifier of the channel or 0 on failure 
///
/// @snippet examples.c param_to_channel
    int param_to_channel (const struct context_rmcios *context,
                          enum type_rmcios paramtype,
                          const union param_rmcios param, int index);

/// @brief Convert parameter to channel function identifier
/// 
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of @p param array
/// @param param array of parameters
/// @param index of the parameter to be converted as identifier
/// @return indentifier of the function or 0 on failure 
//
/// @snippet examples.c param_to_integer
    int param_to_function (const struct context_rmcios *context,
                           enum type_rmcios paramtype,
                           const union param_rmcios param, int index);

/// @brief Get parameters string form length in characters.
/// 
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of parameter array @p param
/// @param param parameter array of type @p paramtype
/// @param index item from array to be read
///
/// @snippet examples.c param_string_length
    int param_string_length (const struct context_rmcios *context,
                             enum type_rmcios paramtype,
                             const union param_rmcios param, int index);

/// @brief Get parameters buffer form payload length in bytes.
///
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of parameter array @p param
/// @param param parameter array of type @p paramtype
/// @param index item from array to be read
/// @return length of parameter represented in buffer
///
/// @snippet examples.c param_buffer_length
    int param_buffer_length (const struct context_rmcios *context,
                             enum type_rmcios paramtype,
                             const union param_rmcios param, int index);

/// @brief Get parameters binary form payload length in bytes.
///
/// Helper function for implementing channels
/// @param context pointer to target system context
/// @param paramtype type of parameter array @p param
/// @param param parameter array of type @p paramtype
/// @param index item from array to be read
/// @return length of parameter represented in buffer
///
/// @snippet examples.c param_buffer_length
    int param_binary_length (const struct context_rmcios *context,
                             enum type_rmcios paramtype,
                             const union param_rmcios param, int index);

/// @brief Get size for allocating needed buffer for string representation. 
/// 
/// Helper function for implementing channels
/// Used together with param_to_string
/// When buffer is not needed returns 0
/// @param context pointer to target system context
/// @param paramtype type of parameter array @p param
/// @param param parameter array of type @p paramtype
/// @param index item from array to be read
/// @return needed buffer size to fit the parameter in string form. 
///     0 when buffer not needed (parameter already in string form)
///
/// @snippet examples.c param_string_alloc_size
    int param_string_alloc_size (const struct context_rmcios *context,
                                 enum type_rmcios paramtype,
                                 const union param_rmcios param, int index);

/// @brief Get size for allocating needed buffer.
/// 
/// Helper function for implementing channels
/// Used together with param_to_buffer
/// When buffer is not needed returns 0
/// Adds single extra byte to returned size.
/// This helps with direct buffer->string compatibility.
/// @param context pointer to target system context
/// @param paramtype type of parameter array @p param
/// @param param parameter array of type @p paramtype
/// @param index item from array to be read
/// @returns needed buffer size +1 to fit the string representation in.
///
/// @snippet examples.c param_buffer_alloc_size
    int param_buffer_alloc_size (const struct context_rmcios *context,
                                 enum type_rmcios paramtype,
                                 const union param_rmcios param, int index);

/// @brief Read data from channel without parameters (float)
/// 
/// @param context pointer to target system context
/// @param channel handle of channel to be read
/// @return float read from the channel
    float read_f (const struct context_rmcios *context, int channel);

/// @brief Reads data from channel without parameter (integer)
///
/// @param context pointer to target system context
/// @param channel handle of channel to be read
/// @return integer read from the channel
    int read_i (const struct context_rmcios *context, int channel);

/// @brief Reads string from channel
///
/// @param context pointer to target system context
/// @param channel handle of channel to be read
/// @param string buffer to read to
/// @param maxlen size of buffer @p string
//  @return required size to fit all data from channel.
    int read_str (const struct context_rmcios *context,
                   int channel, char *string, int maxlen);

/// @brief Write single float value to channel  (float)
///
/// @param context pointer to target system context
/// @param channel handle of channel to be written
/// @param value float value to be written
    float write_f (const struct context_rmcios *context,
                   int channel, float value);

/// @brief Write multiple float parameters to channel.
///
/// @param context pointer to target system context
/// @param channel handle to channel to be written
/// @param params number of parameters to be written from array @p values
    float write_fv (const struct context_rmcios *context,
                    int channel, int params, float *values);

/// @brief Write single integer value to channel
///
/// @param context pointer to target system context
/// @param channel handle to channel to be written
/// @param value integer value to be written
    int write_i (const struct context_rmcios *context,
                 int channel, int value);

/// @brief Write multiple integer parameters to channel.
///
/// @param context pointer to target system context
/// @param channel handle to channel to be written
/// @param params number of parameters to be written from array @p values
/// @param values array of integer value parameters
    int write_iv (const struct context_rmcios *context,
                  int channel, int params, int *values);

/// @brief Write string value to channel, return data to channel
///
/// @param context pointer to target system context
/// @param channel handle to channel
/// @param s string to be written
/// @param return_ch channel handle for return data output
    void write_str (const struct context_rmcios *context,
                    int channel, const char *s, int return_ch);

/// @brief Write ASCII buffer to channel, return data to channel
/// 
/// @param context pointer to target system context
/// @param channel handle to channel
/// @param buffer buffer of data to be written
/// @param length size of @p buffer 
/// @param return_ch channel handle for return data output
    void write_buffer (const struct context_rmcios *context,
                       int channel,
                       const char *buffer, int length, int return_ch);

/// @brief Write binary buffer to channel, return data to uffer
/// 
/// @param context pointer to target system context
/// @param channel handle to channel
/// @param buffer buffer of data to be written
/// @param length size of @p buffer 
/// @param return_data channel pointer to return data buffer
/// @param maxlen size of return_data -buffer
/// @return returns number of bytes fed into the buffer.
    int write_binary (const struct context_rmcios *context,
                      int channel,
                      const char *buffer,
                      int length, char *return_data, int maxlen);

/// @brief get channel link representation handle
/// 
/// @param context pointer to target system context
/// @param channel handle to channel
/// @return Handle for linked channels
    int linked_channels (const struct context_rmcios *context, int channel);

/// Allocate storage from an storage channel
/// @param context pointer to target system context
/// @param size size to allocate. 
/// NOTE: size unit can be different on different storage channels.
/// @param storage_channel channel to allocate from. 
/// Default storage channel: 
/// channel_id:3 allocate memory from default memory allocator (malloc).
/// size=nbytes. return pointer to the allocated memory
/// @return Returns handle to allocated storage.
/// NOTE: Type of handle can be differ on different storage channels. 
/// (pointer,channel handle,record index)
    void *allocate_storage (const struct context_rmcios *context,
                            int size, int storage_channel);

/// Free storage from an storage channel
/// @param context pointer to target system context
/// @param handle previously allocated handle
/// @param storage_channel Channel that was originally used for allocation.
    void free_storage (const struct context_rmcios *context,
                       void *handle, int storage_channel);

/// Convert ASCII name of function into number.
/// @param name name of the function in NULL-terminated ASCII string
/// @return enum number of the function. returns 0 on no match.
    int function_enum (const char *name);

/// Convert ASCII name of channel into channel handle number.
/// @param context pointer to target system context
/// @param channel_name NULL-terminated ascii string.
    int channel_enum (const struct context_rmcios *context,
                      const char *channel_name);

/// Get channel name to buffer.
/// @param context pointer to target system context
/// @param channel_enum channel id number.
/// @param name_to pointer to buffer for the name.
/// @param maxlen size of name_to -buffer
/// @return full size of channel name.
    int channel_name (const struct context_rmcios *context,
                      int channel_id, char *name_to, int maxlen);


// Legacy functions for old-style channel modules:

/// Convert parameter to channel handle or integer. 
///
/// Converts string name into channel handle. And keeps numbers "as it is".
/// Legacy function
/// @param context pointer to target system context
/// @param paramtype type of @p param array
/// @param param array of parameters with type of p paramtype
/// @param index array item to be converted
/// @return channel handle or integer number
    int param_to_int (const struct context_rmcios *context,
                      enum type_rmcios paramtype,
                      const union param_rmcios param, int index);
#ifdef __cplusplus
}

#endif

#endif
