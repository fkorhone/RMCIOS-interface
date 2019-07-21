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
#include "string-conversion.h"

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

int function_detect(const char *name, unsigned int length)
{
   int func_i;
   int i;

   for (func_i = 0; func_i < 16; func_i += 2)
   {
      const char *function = function_enum_pattern[func_i];
      for(i = 0; i < length; i++)
      {
         if (name[i] == 0) break;
         if (name[i] != function[i]) break;
         if (function[i+1] == 0)
         {
            if(name[i+1] == 0 || name[i+1] == ' ' || length == i+1)
            {
               return (int)function_enum_pattern[func_i+1];
            }
         }
      }
   }
   return 0;
}

int function_enum (const char *name)
{
   return function_detect(name, ~0);
}

// ***********************************************************************
// Channelsystem functions that depend only on API interface functions: 
// ***********************************************************************
int create_channel (const struct context_rmcios *context,
                    const char *namebuffer, int namelen,
                    class_rmcios class_func, void *data)
{
   struct buffer_rmcios buffers[3] = {
      {
         .data          = (void *)namebuffer,
         .length        = namelen,
         .size          = 0,
         .required_size = namelen,
         .trailing_size = 0,
      },
      {
         .data          = (void *)&class_func, // function pointer as binary
         .length        = sizeof(class_func),
         .size          = 0,
         .required_size = sizeof(class_func),
         .trailing_size = 0,
      },
      {
         .data          = (void *)&data, // data pointer as binary
         .length        = sizeof(data),
         .size          = 0,
         .required_size = sizeof(data),
         .trailing_size = 0,
      }
   };
   union param_rmcios param = {
      .bv = buffers
   };
   int ireturn;
   struct combo_rmcios returnv=
   {
        .paramtype = int_rmcios,
        .num_params = 1,
        .param.iv = &ireturn,
        .next = 0
   };

   context->run_channel(context,
                        context->create,
                        create_rmcios,
                        binary_rmcios,
                        &returnv,
                        3, param);
   return ireturn;
}

// Create a channel using channel parameters as new channel name.
int create_channel_param (const struct context_rmcios *context,
                          enum type_rmcios paramtype,
                          const union param_rmcios param,
                          int index,
                          class_rmcios channel_function, 
                          void *channel_data)
{
   // Determine the name size:
   int namelen = param_string_length (context, paramtype, param, index);
   // Allocate memory for name:
   char *name = (char *) allocate_storage (context, namelen + 1, 0);
   // Get the name:
   param_to_string (context, paramtype, param, index, namelen + 1, name);

   return create_channel(context, name, namelen, channel_function,
                                   channel_data);
}

void link_channel (const struct context_rmcios *context,
                   int channel, int to_channel)
{
   int params[2] = { channel, to_channel };
   context->run_channel (context, context->link, write_rmcios, int_rmcios,
                         0, 2,
                         (const union param_rmcios) params);
}

/// @brief Link the channel to another channel, specifying the linked functions
/// Linked function can also be specified, or just set to 0 (all functions)
void link_channel_function (const struct context_rmcios *context,
                            int channel,
                            int to_channel,
                            enum function_rmcios function, int to_function)
{
   int params[4] = { channel, function, to_channel, to_function };
   context->run_channel (context, context->link, write_rmcios, int_rmcios,
                         0, 4,
                         (const union param_rmcios) params);
}

// Create a channel using null-terminated string as new channel name
// String should not be temporary variable.
int create_channel_str (const struct context_rmcios *context,
                        const char *channel_name,
                        class_rmcios channel_function, void *channel_data)
{
   int len;
   for (len = 0; channel_name[len] != 0; len++);        // look length of name
   return create_channel(context, channel_name, len,
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
      return 0; // Cannot create subchannel without suffix.
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
   switch (returnv->paramtype)
   {
   case float_rmcios:
      returnv->param.fv[0] = value;
      break;
   case int_rmcios:
      returnv->param.iv[0] = value;
      break;
   case channel_rmcios:
      context->run_channel (context, returnv->param.channel, write_rmcios,
                            int_rmcios, 0, 1,
                            (const union param_rmcios) &value);
      break;
   case buffer_rmcios:
      {
         struct buffer_rmcios *sreturn = returnv->param.bv;
         int n = sreturn->size - sreturn->length;
         int rsize; 
         if (n > 0)
         {
            char *end = sreturn->data + sreturn->length ;
            rsize= integer_to_string (end, n, value) ;
            for (n = 0; end[n] != 0; n++);
            sreturn->length += n;
         }
         else rsize=integer_to_string(0,0,value) ;
         sreturn->required_size = rsize;
         sreturn->trailing_size = 1 ;
         break;
      }
   case binary_rmcios:
      {
         struct buffer_rmcios *sreturn = returnv->param.bv;
         if ((sreturn->size - sreturn->length) >= sizeof (value))
         {
            *((int *) (sreturn->data + sreturn->length)) = value;
            sreturn->length += sizeof (value);
         }
         sreturn->required_size = sizeof (value);
      }
      break;
   case combo_rmcios:
      {
         struct combo_rmcios *creturn = returnv->param.cv;
         return return_int (context, creturn, value);
      }
      break;
   }
}

void return_float (const struct context_rmcios *context,
                   struct combo_rmcios *returnv, float value)
{
   if (returnv == 0 || returnv->num_params == 0)
   {
      return;
   }
   switch (returnv->paramtype)
   {
   case float_rmcios:
      returnv->param.fv[0] = value;
      break;
   case int_rmcios:
      returnv->param.iv[0] = value;
      break;
   case channel_rmcios:
      context->run_channel (context, returnv->param.channel, write_rmcios,
                            float_rmcios, 0, 1,
                            (const union param_rmcios) &value);
      break;
   case buffer_rmcios:
      {
         struct buffer_rmcios *sreturn = returnv->param.bv;
         int n = sreturn->size - sreturn->length;
         int rsize ;
         if (n > 0)
         {
            char *end = sreturn->data + sreturn->length;
            rsize=float_to_string (sreturn->data + sreturn->length, n, value);
            for (n = 0; end[n] != 0; n++);
            sreturn->length += n;
         }
         else rsize=float_to_string(0,0,value) ;

         sreturn->required_size = rsize ;
         // Tell that is space for terminating null after data.
         sreturn->trailing_size = 1 ;
      }
      break;

   case binary_rmcios:
      {
         struct buffer_rmcios *sreturn = returnv->param.bv;
         if ((sreturn->size - sreturn->length) > sizeof (value))
         {
            *((float *) (sreturn->data + sreturn->length)) = value;
            sreturn->length += sizeof (value);
         }
         sreturn->required_size = sizeof (value);
      }
      break;
   case combo_rmcios:
      {
         struct combo_rmcios *creturn = returnv->param.cv;
         return return_float (context, creturn, value);
      }
      break;
   }
}

void return_string (const struct context_rmcios *context,
                    struct combo_rmcios *returnv, const char *string)
{
   int i = 0, si = -1;

   if (returnv == 0 || returnv->num_params == 0)
   {
      return;
   }
   switch (returnv->paramtype)
   {
   case float_rmcios:
      returnv->param.fv[0] = string_to_float (string);
      break;
   case int_rmcios:
      returnv->param.iv[0] = string_to_integer (string);
      break;
   case channel_rmcios:
      {
         struct buffer_rmcios send_buff;
         send_buff.data = (char *) string;
         // get length of string
         for (i = 0; string[i] != 0; i++);      
         // payload length:
         send_buff.length = i;  
         // size includes the NULL terminator
         send_buff.size = i + 1;        
         // required size is size of data
         send_buff.required_size = i;
         // There is a trailing terminating zero
         send_buff.trailing_size = 1 ;   
         context->run_channel (context, returnv->param.channel, write_rmcios,
                               buffer_rmcios, 0, 1,
                               (const union param_rmcios) &send_buff);
      }
      break;
   case buffer_rmcios: // copy data to buffer
   case binary_rmcios:
      {
         struct buffer_rmcios *sreturn = returnv->param.bv;
         for (i = sreturn->length; i < sreturn->size; i++)
         {
            // append string (include terminating zero character)
            sreturn->data[i] = string[++si];       
            if (string[si] == 0)
               break;      // end of string 
         }
         if (si < 0) si = 0;
         if (string[si] != 0)
         {
            while (string[++si] != 0);
         }
         // Required size is complete size of data without terminating zero.
         sreturn->required_size = si ;
         if(si < sreturn->size )
            // String fit in the buffer
         {
            // Length is size of data excluding terminating zero
            sreturn->length = i-1;
            // Terminating zero is marked as trailing character after the data
            sreturn->trailing_size = 1 ;
         }
         else 
            // String did not fit completely in the buffer.
         {
            // Length is the ammount of data that fit in the buffer
            sreturn->length = i ;
            // There is no space for terminating zero:
            sreturn->trailing_size = 0 ;
         }
      }
      break;
   case combo_rmcios:
      {
         struct combo_rmcios *creturn = returnv->param.cv;
         return return_string (context, creturn, string);
      }
      break;
   }
}

void return_buffer (const struct context_rmcios *context,
                    struct combo_rmcios *returnv,
                    const char *buffer, unsigned int length)
{
   int i = 0, si = 0;

   if (returnv == 0 || returnv->num_params == 0)
   {
      return;
   }
   switch (returnv->paramtype)
   {
   case float_rmcios:
      {
         char string[length + 1];
         for (i = 0; i < length; i++)
            string[i] = buffer[i];
         string[length] = 0;
         returnv->param.fv[0] = string_to_float (string);
      }
      break;
   case int_rmcios:
      {
         char string[length + 1];
         for (i = 0; i < length; i++)
            string[i] = buffer[i];
         string[length] = 0;
         returnv->param.iv[0] = string_to_integer (string);
      }
      break;
   case channel_rmcios:
      {
         struct buffer_rmcios sreturn;
         sreturn.length = length;
         sreturn.size = 0;
         sreturn.required_size = length;
         sreturn.trailing_size = 0;
         sreturn.data = (char *) buffer;
         context->run_channel (context, returnv->param.channel, write_rmcios, 
                                 buffer_rmcios, 0, 1, 
                                 (const union param_rmcios) &sreturn) ;       
      }
      break;
   case buffer_rmcios:
   case binary_rmcios:
      {
         struct buffer_rmcios *sreturn = returnv->param.bv;
         for (i = sreturn->length; i < sreturn->size && si < length; i++)
         {
            // append data to buffer
            sreturn->data[i] = buffer[si++];    
         }
         sreturn->length = i;
         sreturn->trailing_size = 0;
         sreturn->required_size = length;
      }
      break;
   case combo_rmcios:
      {
         struct combo_rmcios *creturn = returnv->param.cv;
         return return_buffer (context, creturn, buffer, length);
      }
      break;
   }
}

void return_binary (const struct context_rmcios *context,
                    struct combo_rmcios *returnv,
                    const char *buffer, unsigned int length)
{
   int i = 0, si = 0;
   if (returnv == 0 || returnv->num_params == 0)
   {
      return;
   }
   switch (returnv->paramtype)
   {
   case float_rmcios:
      {
         char *d = (char *) returnv->param.p;
         for (i = 0; i < sizeof (float) && i < length; i++)
            d[i] = buffer[i];
      }
      break;
   case int_rmcios:
      {
         char *d = (char *) returnv->param.p;
         for (i = 0; i < sizeof (int) && i < length; i++)
            d[i] = buffer[i];
      }
      break;
   case channel_rmcios:
      {
         struct buffer_rmcios sreturn;
         sreturn.length = length;
         sreturn.size = length;
         sreturn.required_size=length;
         sreturn.trailing_size=0 ;
         sreturn.data = (char *) buffer;
         // execute write to return channel
         context->run_channel (context, returnv->param.channel, write_rmcios, 
                               binary_rmcios, 0, 1, 
                               (const union param_rmcios) &sreturn);        
      }
      break;
   case buffer_rmcios:
   case binary_rmcios:
      {
         struct buffer_rmcios *sreturn = returnv->param.bv;
         for (i = sreturn->length; i < sreturn->size && si < length; i++)
         {
            // append data to buffer
            sreturn->data[i] = buffer[si++];    
         }
         sreturn->length = i;
         sreturn->required_size = length;
         sreturn->trailing_size = 0;
      }
      break;
   case combo_rmcios:
      {
         struct combo_rmcios *creturn = returnv->param.cv;
         return return_binary (context, creturn, buffer, length);
      }
      break;
   }
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
      context->run_channel (context, returnv->param.channel, write_rmcios,
                            buffer_rmcios, 0, 0,
                            (const union param_rmcios) 0);
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
   float returnv = 0.0 / 0.0;   // NAN
   int len;
   if (params.p == 0)
   {
      return returnv;
   }
   
   switch (paramtype)
   {
   case float_rmcios:
      returnv = params.fv[index];
      break;
   case int_rmcios:
      returnv = params.iv[index];
      break;
   case buffer_rmcios:
      // get space needed for string
      len = param_string_alloc_size (context, paramtype, params, index);        
      {
         // allocate space for string
         char buffer[len];      
         const char *s;
         // convert to string
         s = param_to_string (context, paramtype, params, index, len, buffer);  
         returnv = string_to_float (s);
      }
      break;
   case binary_rmcios:
      {
         struct buffer_rmcios p = params.bv[index];
         char *d = (char *) &returnv;
         int i;
         for (i = 0; i < sizeof (returnv) && i < p.length; i++)
         {
            d[i] = p.data[i];
         }
      }
      break;
   case combo_rmcios:
      {
         struct combo_rmcios *p = params.cv;
         while (index >= p->num_params)
         {
            index -= p->num_params;
            if (p->next == 0)
               p++;
            else
               p = p->next;
         }
         param_to_float (context, p->paramtype, p->param, index);
      }
      break;
   }
   return returnv;
}

int param_to_integer (const struct context_rmcios *context,
                      enum type_rmcios paramtype,
                      const union param_rmcios params, int index)
{
   int returnv = 0;
   int len;
   if (params.cp == 0)
      return returnv;
   switch (paramtype)
   {
   case float_rmcios:
      returnv = params.fv[index];
      break;
   case int_rmcios:
      returnv = params.iv[index];
      break;
   case buffer_rmcios:
      // get space needed for string
      len = param_string_alloc_size (context, paramtype, params, index); 
      {
         // allocate space for string
         char buffer[len];  
         const char *s;
         // convert to string
         s = param_to_string (context, paramtype, params, index, len, buffer);  
         returnv = string_to_integer (s);   
      }
      break;

   case binary_rmcios:
      {
         struct buffer_rmcios p = params.bv[index];
         char *d = (char *) &returnv;
         int i;
         for (i = 0; i < sizeof (returnv) && i < p.length; i++)
         {
            d[i] = p.data[i];
         }
      }
      break;
   case combo_rmcios:
      {
         struct combo_rmcios *p = params.cv;
         while (index >= p->num_params)
         {
            index -= p->num_params;
            if (p->next == 0)
               p++;
            else
               p = p->next;
         }
         return param_to_integer (context, p->paramtype,
                                  (const union param_rmcios) p->param, index);
      }
      break;
   }
   return returnv;
}

int param_to_channel (const struct context_rmcios *context,
                  enum type_rmcios paramtype,
                  const union param_rmcios params, int index)
{
   int returnv = 0;
   int ch_enum;
   int len;
   if (params.p == 0)
      return returnv;
   switch (paramtype)
   {
   case float_rmcios:
      returnv = params.fv[index];
      break;
   case int_rmcios:
      returnv = params.iv[index];
      break;
   case buffer_rmcios:
   case binary_rmcios:
      // get space needed for string
      len = param_string_alloc_size (context, paramtype, params, index);        
      {
         // allocate space for string
         char buffer[len];      
         const char *s;
         // convert to string
         s = param_to_string (context, paramtype, params, index, len, buffer);  
         ch_enum = channel_enum (context, s);   
         // set return value to channel enum
         returnv = ch_enum;  
      }
      break;
   case combo_rmcios:
      {
         struct combo_rmcios *p = params.cv;
         while (index >= p->num_params)
         {
            index -= p->num_params;
            if (p->next == 0)
               p++;
            else
               p = p->next;
         }
         return param_to_channel (context, p->paramtype, p->param, index);
      }
      break;

   }
   return returnv;
}

int param_to_int (const struct context_rmcios *context,
                  enum type_rmcios paramtype,
                  const union param_rmcios params, int index)
{
   int returnv = 0;
   int ch_enum;
   int len;
   if (params.p == 0)
      return returnv;
   switch (paramtype)
   {
   case float_rmcios:
      returnv = params.fv[index];
      break;
   case int_rmcios:
      returnv = params.iv[index];
      break;
   case buffer_rmcios:
   case binary_rmcios:
      // get space needed for string
      len = param_string_alloc_size (context, paramtype, params, index);        
      {
         // allocate space for string
         char buffer[len];      
         const char *s;
         // convert to string
         s = param_to_string (context, paramtype, params, index, len, buffer);  
         ch_enum = channel_enum (context, s);   
         // check if channel name
         if (ch_enum != 0)
         // set return value to channel enum
            returnv = ch_enum;  
         else
            returnv = string_to_float (s);      
      }
      break;
   case combo_rmcios:
      {
         struct combo_rmcios *p = params.cv;
         while (index >= p->num_params)
         {
            index -= p->num_params;
            if (p->next == 0)
               p++;
            else
               p = p->next;
         }
         return param_to_int (context, p->paramtype, p->param, index);
      }
      break;

   }
   return returnv;
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
   struct buffer_rmcios p_buffer;
   int length;

   // End buffer with NULL to prevent memory overflow on simultanous read&write
   if (maxlen > 0)
      to_str[maxlen - 1] = 0;   // Terminate the buffer with NULL

   if (params.p == 0)
      return 0;
   switch (paramtype)
   {
   case float_rmcios:
      float_to_string (to_str, maxlen, params.fv[index]);
      return to_str;

   case int_rmcios:
      integer_to_string (to_str, maxlen, params.iv[index]);
      return to_str;
   
   case buffer_rmcios:
   case binary_rmcios:

      p_buffer = params.bv[index];

      // copy string up to maxlen characters:
      length = copy_mem_safe (p_buffer.data, p_buffer.length, to_str, maxlen);
      if (maxlen > 0)
      {
         if (length < maxlen)
            // Terminate data with NULL
            to_str[length] = 0; 
         else 
            // Terminate buffer with NULL
            to_str[maxlen - 1] = 0;      
      }

      if (p_buffer.trailing_size > 0 && p_buffer.data[p_buffer.length] == 0)
      // The parameter contain trailing null after data.
      {
         return p_buffer.data;  // return the original buffer pointer
      }
      else      
      // No null terminator -> return user buffer
      {
         return to_str; // Return the copied data
      }
   case combo_rmcios:
      {
         struct combo_rmcios *p = params.cv;
         while (index >= p->num_params)
         {
            index -= p->num_params;
            if (p->next == 0)
               p++;
            else
               p = p->next;
         }
         return param_to_string (context, p->paramtype, p->param, index,
                                 maxlen, to_str);
      }
      break;
   }
   return 0;
}

struct buffer_rmcios param_to_buffer (const struct context_rmcios *context,
                                      enum type_rmcios paramtype,
                                      const union param_rmcios params,
                                      int index, int maxlen, char *to_str)
{
   struct buffer_rmcios rbuffer;
   struct buffer_rmcios p_buffer;
   int i;
   int len;
   if (params.p == 0)
   {
      rbuffer.length = 0;
      rbuffer.size = 0;
      rbuffer.data = 0;
      return rbuffer;
   }
   switch (paramtype)
   {
   case float_rmcios:
   case int_rmcios:

      if (paramtype == float_rmcios)
         len = float_to_string (to_str, maxlen, params.fv[index]);
      else
         len = integer_to_string (to_str, maxlen, params.iv[index]);
      // copying also last char
      if (len >= maxlen)
      { // Check if buffer becomes full
         char tmp[maxlen + 1];
         if (paramtype == float_rmcios)
            float_to_string (tmp, sizeof (tmp), params.fv[index]);
         else
            integer_to_string (tmp, sizeof (tmp), params.iv[index]);
         // add also the last char                      
         to_str[maxlen - 1] = tmp[maxlen - 1];  
      }
      rbuffer.data = to_str;
      for (i = 0; to_str[i] != 0; i++); // get string length
      if (maxlen > 0)
      {
         rbuffer.length = i;    // payload size
         rbuffer.size = maxlen;
      }
      else
         rbuffer.length = 0;
      return rbuffer;

   case buffer_rmcios:
   case binary_rmcios:
      p_buffer = params.bv[index];

      // Copy data to user buffer
      int length;
      length = copy_mem_safe (p_buffer.data, p_buffer.length, to_str, maxlen);       
      if (length < maxlen)
         // add extra NULL terminator for string compatibility
         to_str[length] = 0;    


         // Return the orginal parameter data
         return params.bv[index];       
   case combo_rmcios:
      {
         struct combo_rmcios *p = params.cv;
         while (index >= p->num_params)
         {
            index -= p->num_params;
            if (p->next == 0)
               p++;
            else
               p = p->next;
         }
         return param_to_buffer (context, p->paramtype, p->param, index,
                                 maxlen, to_str);
      }
      break;
   }
   rbuffer.length = 0;
   rbuffer.size = 0;
   rbuffer.data = 0;
   return rbuffer;
}

struct buffer_rmcios param_to_binary (const struct context_rmcios *context,
                                      enum type_rmcios paramtype,
                                      const union param_rmcios params,
                                      int index, int maxlen, void *buffer)
{
   struct buffer_rmcios rbuffer;
   struct buffer_rmcios p_buffer;
   if (params.p == 0)
   {
      rbuffer.length = 0;
      rbuffer.size = 0;
      rbuffer.data = 0;
      return rbuffer;
   }
   switch (paramtype)
   {
   case float_rmcios:
      rbuffer.size = 0;
      rbuffer.length = sizeof (float);
      rbuffer.data = (char *) &(params.fv[index]);
      if (maxlen >= sizeof (float))
         *((float *) buffer) = params.fv[index];
      return rbuffer;

   case int_rmcios:
      rbuffer.size = 0;
      rbuffer.length = sizeof (int);
      rbuffer.data = (char *) &(params.iv[index]);
      if (maxlen >= sizeof (int))
         *((int *) buffer) = params.iv[index];
      return rbuffer;

   case buffer_rmcios:
   case binary_rmcios:
      p_buffer = params.bv[index];

      // Copy data to user buffer
      copy_mem_safe (p_buffer.data, p_buffer.length, 
                     buffer, maxlen);

      // Return the orginal parameter data
      return params.bv[index]; 

   case combo_rmcios:
      {
         struct combo_rmcios *p = params.cv;
         while (index >= p->num_params)
         {
            index -= p->num_params;
            if (p->next == 0)
               p++;
            else
               p = p->next;
         }
         return param_to_binary (context, p->paramtype, p->param, index,
                                 maxlen, buffer);
      }
      break;
   }
   rbuffer.length = 0;
   rbuffer.size = 0;
   rbuffer.data = 0;
   return rbuffer;
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
         struct buffer_rmcios fname;
         fname = param_to_buffer (context, paramtype, 
                                  param, index, 
                                  blen, buffer);

         function = function_detect(fname.data, fname.length);
      }
   }
   return function;
}

int param_string_length (const struct context_rmcios *context,
                         enum type_rmcios paramtype,
                         const union param_rmcios param, int index)
{
   struct buffer_rmcios p_buffer = param.bv[index];
   int len;
   switch (paramtype)
   {
   case float_rmcios:
      return float_to_string (0, 0, param.fv[index]);
   case int_rmcios:
      return integer_to_string (0, 0, param.iv[index]);
   case buffer_rmcios:
   case binary_rmcios:
      for (len = 0; len < p_buffer.length && p_buffer.data[len] != 0; len++);
      // Return actual string length (in characters)
      return len;       
   case combo_rmcios:
      {
         struct combo_rmcios *p = param.cv;
         while (index >= p->num_params)
         {
            index -= p->num_params;
            if (p->next == 0)
               p++;
            else
               p = p->next;
         }
         return param_string_length (context, p->paramtype, p->param, index);
      }
      break;
   }
   return 0;
}

int param_buffer_length (const struct context_rmcios *context,
                         enum type_rmcios paramtype,
                         const union param_rmcios param, int index)
{
   struct buffer_rmcios p_buffer = param.bv[index];
   switch (paramtype)
   {
   case float_rmcios:
      return float_to_string (0, 0, param.fv[index]);
   case int_rmcios:
      return integer_to_string (0, 0, param.iv[index]);
   case buffer_rmcios:
   case binary_rmcios:
      // Return actual buffer length
      return p_buffer.length;   
   case combo_rmcios:
      {
         struct combo_rmcios *p = param.cv;
         while (index >= p->num_params)
         {
            index -= p->num_params;
            if (p->next == 0)
               p++;
            else
               p = p->next;
         }
         return param_buffer_length (context, p->paramtype, p->param, index);
      }
   }
   return 0;
}

int param_binary_length (const struct context_rmcios *context,
                         enum type_rmcios paramtype,
                         const union param_rmcios param, int index)
{
   struct buffer_rmcios p_buffer = param.bv[index];
   switch (paramtype)
   {
   case float_rmcios:
      return sizeof (float);
   case int_rmcios:
      return sizeof (int);
   case buffer_rmcios:
   case binary_rmcios:
      return p_buffer.length;
   case combo_rmcios:
      {
         struct combo_rmcios *p = param.cv;
         while (index >= p->num_params)
         {
            index -= p->num_params;
            if (p->next == 0)
               p++;
            else
               p = p->next;
         }
         return param_binary_length (context, p->paramtype, p->param, index);
      }
   }
   return 0;
}

int param_string_alloc_size (const struct context_rmcios *context,
                             enum type_rmcios paramtype,
                             const union param_rmcios param, int index)
{
   struct buffer_rmcios p_buffer = param.bv[index];
   int len;
   switch (paramtype)
   {
   case float_rmcios:
   case int_rmcios:
      return 20;
   case buffer_rmcios:
   case binary_rmcios:
      if (p_buffer.trailing_size > 0 && p_buffer.data[p_buffer.length] == 0)
         // check if buffer contains null-terminated string
      {
         return 0;
         // Buffer contains NULL-terminator after payload -> no need to allocate
      }
      // strlen :
      for (len = 0; len < p_buffer.length && p_buffer.data[len] != 0; len++);  
      // need to allocate strlen+1
      return len + 1;   
   case combo_rmcios:
      {
         struct combo_rmcios *p = param.cv;
         while (index >= p->num_params)
         {
            index -= p->num_params;
            if (p->next == 0)
               p++;
            else
               p = p->next;
         }
         return param_string_alloc_size (context, p->paramtype, p->param,
                                         index);
      }

   }
   return 0;
}

int param_buffer_alloc_size (const struct context_rmcios *context,
                             enum type_rmcios paramtype,
                             const union param_rmcios param, int index)
{
   switch (paramtype)
   {
   case float_rmcios:
   case int_rmcios:
      return 20;
   case buffer_rmcios:
   case binary_rmcios:
         return 0 ; // No need to allocate any memory
   case combo_rmcios:
      {
         struct combo_rmcios *p = param.cv;
         while (index >= p->num_params)
         {
            index -= p->num_params;
            if (p->next == 0)
               p++;
            else
               p = p->next;
         }
         return param_buffer_alloc_size (context, p->paramtype, p->param,
                                         index);
      }
   }

   return 0;
}

float read_f (const struct context_rmcios *context, int channel)
{
   float rvalue = 0;
   struct combo_rmcios returnv = {
      .paramtype = float_rmcios,
      .num_params = 1,
      .param.fv = &rvalue
   };
   context->run_channel (context, channel,
                         read_rmcios, float_rmcios,
                         &returnv,
                         0, (const union param_rmcios) 0);
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
   context->run_channel (context, channel,
                         read_rmcios, int_rmcios,
                         &returnv,
                         0, (const union param_rmcios) 0);
   return rvalue;
}

int read_str (const struct context_rmcios *context,
               int channel, char *string, int maxlen)
{
   struct buffer_rmcios sreturn;
   struct combo_rmcios returnv = {
      .paramtype = buffer_rmcios,
      .num_params = 1,
      .param.bv = &sreturn
   };

   if(maxlen!=0) sreturn.size = maxlen - 1;
   else sreturn.size = 0 ;
   sreturn.data = string;
   sreturn.length = 0;
   context->run_channel (context, channel,
                         read_rmcios, buffer_rmcios,
                         &returnv,
                         0, (const union param_rmcios) 0);
   if(sreturn.size!=0) sreturn.data[sreturn.length] = 0;    // Add NULL-termination
   return sreturn.required_size ;
}

float write_f (const struct context_rmcios *context, int channel, float value)
{
   float rvalue = 0;
   struct combo_rmcios returnv = {
      .paramtype = float_rmcios,
      .num_params = 1,
      .param.fv = &rvalue
   };
   context->run_channel (context, channel,
                         write_rmcios, float_rmcios,
                         &returnv,
                         1, (const union param_rmcios) &value);
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
   context->run_channel (context, channel,
                         write_rmcios, float_rmcios,
                         &returnv,
                         params, (const union param_rmcios) values);
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
   context->run_channel (context, channel,
                         write_rmcios, int_rmcios,
                         &returnv,
                         params, (const union param_rmcios) values);
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
   context->run_channel (context, channel, write_rmcios, int_rmcios,
                         &returnv, 1,
                         (const union param_rmcios) &value);
   return rvalue;
}

void write_str (const struct context_rmcios *context,
                int channel, const char *str, int channel_id)
{
   struct buffer_rmcios param;
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
   param.trailing_size = 1;  // Trailing 0
   
   context->run_channel (context, channel,
                         write_rmcios, buffer_rmcios,
                         &returnv,
                         1, (const union param_rmcios) &param);
}

void write_buffer (const struct context_rmcios *context, int channel,
                   const char *buffer, int length, int channel_id)
{
   struct buffer_rmcios param;
   struct combo_rmcios returnv = {
      .paramtype = channel_rmcios,
      .num_params = 1,
      .param.channel = channel_id,
   };
   param.data = (char *) buffer;
   param.length = length;   
   param.size = 0;
   param.required_size = length;
   param.trailing_size = 0;

   context->run_channel (context, channel,
                         write_rmcios, buffer_rmcios,
                         &returnv,
                         1, (const union param_rmcios) &param);
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
   context->run_channel (context, channel,
                         write_rmcios, binary_rmcios,
                         &returnv,
                         1, (const union param_rmcios) &param);

   return breturnv.required_size;
}

int linked_channels (const struct context_rmcios *context, int channel)
{
   int ireturn;
   struct combo_rmcios returnv = {
      .paramtype = int_rmcios,
      .num_params = 1,
      .param.iv = &ireturn
   };
   context->run_channel (context, context->link,
                         read_rmcios, int_rmcios,
                         &returnv,
                         1, (const union param_rmcios) &channel);
   return ireturn;
}

void *allocate_storage (const struct context_rmcios *context, int size,
                        int storage_channel)
{

   struct buffer_rmcios breturnv;
   struct combo_rmcios returnv = {
      .paramtype = buffer_rmcios,
      .num_params = 1,
      .param.bv = &breturnv
   };
   struct buffer_rmcios param;
   param.data = (void *) &size;
   param.length = sizeof (size);
   param.size = 0;
   void *ptr;
   breturnv.data = (void *) &ptr;
   breturnv.length = 0;
   breturnv.size = sizeof (ptr);

   if (storage_channel == 0)
      storage_channel = context->mem;
   //return (void *) write_i(storage_channel, size) ;
   context->run_channel (context, storage_channel,
                         write_rmcios, binary_rmcios,
                         &returnv,
                         1, (const union param_rmcios) &param);
   return ptr;
}

void free_storage (const struct context_rmcios *context, void *ptr,
                   int storage_channel)
{
   struct buffer_rmcios param[2];
   param[0].data = 0;
   param[0].size = 0;
   param[0].length = 0;
   param[1].data = (char *) &ptr;
   param[1].size = 0;
   param[1].length = sizeof (ptr);
   if (storage_channel == 0) storage_channel = context->mem;

   context->run_channel (context, storage_channel, write_rmcios,
                         binary_rmcios, 0, 2,
                         (const union param_rmcios) param);
}

int channel_enum (const struct context_rmcios *context,
                  const char *channel_name)
{
   int ireturn;
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
   param.size = 0;
   context->run_channel (context, context->id,
                         read_rmcios, buffer_rmcios,
                         &returnv,
                         1, (const union param_rmcios) &param);
   return ireturn;
}

int channel_name (const struct context_rmcios *context,
                  int channel_id, char *name_to, int maxlen)
{
   struct buffer_rmcios breturnv = {
      .data = name_to,
      .length = 0,
      .size = maxlen
   };
   struct combo_rmcios returnv = {
      .paramtype = buffer_rmcios,
      .num_params = 1,
      .param.bv = &breturnv
   };
   context->run_channel (context, context->name,
                         read_rmcios, int_rmcios,
                         &returnv,
                         1, (const union param_rmcios) &channel_id);
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
   // size of payload
   pb.length = i;       
   // writable size
   pb.size = 0; 
   pb.required_size = i;
   pb.trailing_size = 1;
   return pb;
}

struct buffer_rmcios make_const_buffer (const char *buffer, int length)
{
   struct buffer_rmcios pb;
   pb.data = (char *) buffer;
   // size of payload
   pb.length = length;  
   // writable size
   pb.size = 0; 
   pb.required_size = length;
   pb.trailing_size = 0;
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
   // size of payload
   pb.length = i;       
   // writable size
   pb.size = size; 
   pb.required_size = i;
   pb.trailing_size = 0;
   return pb;
}

struct buffer_rmcios make_buffer (char *buffer, int length)
{
   struct buffer_rmcios pb;
   pb.data = (char *) buffer;
   // size of payload
   pb.length = length;  
   // writable size
   pb.size = length;    
   pb.required_size = length;
   pb.trailing_size = 0;
   return pb;
}

