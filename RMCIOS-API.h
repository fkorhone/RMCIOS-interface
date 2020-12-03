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
 * @file channel_api.h
 * @author Frans Korhonen
 * @brief channel system application interface.
 */
#ifndef channel_api_h
#define channel_api_h

/// @brief Channel function (class_rmcios ) parameter types
    enum type_rmcios
    {
        /// Parameters as signed integers.
        int_rmcios = 1,

        /// System float point Parameters as floats. 
        float_rmcios = 2,

        /// buffer_rmcios structure containing size and pointer to buffer data.
        /// Return data is copied to address found from buffer_rmcios structure.
        /// Ammount of bytes copied is stored in the length variable.
        buffer_rmcios = 3,

        /// Channel identifier
        channel_rmcios = 4,

        /// Raw Binary buffer
        /// Parameters are raw binary arrays. 
        /// Capsuled inside buffer_rmcios structures. 
        binary_rmcios = 5,

        /// Variable type parameters
        /// Parameters are combo_rmcios structures
        combo_rmcios = 6
    };

/// @brief channel functions 
    enum function_rmcios
    {
        /// Runtime help
        help_rmcios = 1,
        /// Setup channel specific parameters
        setup_rmcios,
        /// Write data to channel
        write_rmcios,
        /// Read data from channel
        read_rmcios,
        /// Create new channel from called channel
        create_rmcios,
        /// Link channel to another channel 
        link_rmcios
    };

/// @brief Structure for buffers
    struct buffer_rmcios
    {
        /// Data pointer
        char *data;
        /// Length of data in the buffer
        unsigned int length;
        /// Modifiable buffer size. Set to 0 on read only buffers.
        unsigned int size;
        /// Size required to completely fit original data.
        /// Used to determine the needed size for buffers.
        unsigned int required_size;
        
        /// Number of accessible metabytes after the buffer
        /// eg. NULL -terminated string buffer contain 1 trailing metabyte (0)
        unsigned short int trailing_size;
    } ;

    struct combo_rmcios;

/// @brief Union for channel class function parameters
    union param_rmcios
    {
        /// Direct pointer to parameters:
        void *p;
        /// Direct constant pointer to parameters:
        const void *cp;
        /// paramtype==int_rmcios
        int *iv;
        /// paramtype==float_rmcios
        float *fv;
        /// paramtype==buffer_rmcios
        struct buffer_rmcios *bv;
        /// paramtype==combo_rmcios
        struct combo_rmcios *cv;
        /// paramtype==channel_rmcios
        int channel;
    };

/// @brief Linked list structure of parameter arrays.
// for combined and variable parameter types.
    struct combo_rmcios
    {
        /// Type of parameter array
        enum type_rmcios paramtype;
        /// Number of parameters in the parameter array
        int num_params;
        /// Pointer to parameter array.
        const union param_rmcios param;
        /// Pointer to next parameter array. Set to 0 on last. 
        struct combo_rmcios *next;
    };

    struct context_rmcios;

/// @brief Typedef for channel callback funtion pointer. 
/// Easier casting of channel function pointers. 
/// @param data Pointer to channel member data.
/// @param context Pointer to executing rmcios system context. 
/// @param id Id of the channel in the executing rmcios context.
/// @param function RMCIOS function that is called on the channel.
/// @param paramtype Type of returnv and parameters
/// @param returnv Pointer to return variable. 
///        Type of variable is defined by @p paramtype.
/// @param num_params Number of parameters
/// @param param pointer to array of parameters. 
//         Type and size of array items are defined by @p paramtype.
    typedef void (*class_rmcios) (void *data,
                                  const struct context_rmcios * context,
                                  int id,
                                  enum function_rmcios function,
                                  enum type_rmcios paramtype,
                                  struct combo_rmcios *returnv,
                                  int num_params,
                                  const union param_rmcios param);

/// @brief context structure for delivering api interface to modules
/// @param version
    struct context_rmcios
    {
        /// Context version 
        int version;

        /// Function pointer for channel implementation
        class_rmcios run_channel;

        /// Data pointer for context implementation
        void *data;
        /// Channel for reading channel identifier number.
        int id;
        /// Channel for reading channel name
        int name;
        /// Channel for allocating memory.
        int mem;
        /// Channel for allocating temporary memory. 
        /// Mainly for data exchange between channels.
        int quemem;
        /// Channel for error messages.
        int errors;
        /// Channel for warning messages.
        int warning;
        /// Channel for reporting. eg. module initialization messages etc.
        int report;
        /// Channel for text based control.
        int control;
        /// Channel for creating links
        int link;
        /// Channel for interacting with linked channels
        int linked;
        /// Channel for creating new channels
        int create;
        /// Channel For converting parameters
        int convert;
    };

#endif
