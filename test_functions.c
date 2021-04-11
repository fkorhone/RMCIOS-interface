#include <stdio.h>
#include <string.h>

#include "test.h"
#include "RMCIOS-API.h"
#include "RMCIOS-functions.c"

#define TEST_FUNC_NAME run_stub
#define TEST_CALLBACK_NAME run_callback
#undef TEST_FUNC_RETURN_TYPE
#define TEST_FUNC_PARAMS PARAM(void *, data) SEP\
                         PARAM(const struct context_rmcios * ,context) SEP\
                         PARAM(int, id) SEP \
                         PARAM(enum function_rmcios, function) SEP \
                         PARAM(enum type_rmcios, paramtype) SEP \
                         PARAM(struct combo_rmcios *, returnv) SEP\
                         PARAM(int, num_params) SEP \
                         PARAM(union param_rmcios, param)
#include "test_callback_template.h"

struct context_rmcios context_mock =
{
        .run_channel = run_stub,
        .id = 55,
        .name = 56,
        .mem = 57,
        .quemem = 58,
        .errors = 59,
        .warning = 60,
        .report = 61,
        .control = 62,
        .link = 63,
        .linked = 64,
        .create = 65,
        .convert = 66,
};

TEST_RUNNER
{
    TEST_SUITE("create_channel")
    { 
        SUITE_SETUP()

        TEST_CASE("no_name", "Create channel without name")
        {
            static int new_channel_id = 513;
            static int class_address = 1555;
            static int data_address = 2013;

            TEST_CALLBACK(run_callback)
            {
                // Runs context.create to create the channel 
                TEST_ASSERT_EQUAL_INT(run_callback.id, context_mock.create);
                TEST_ASSERT_EQUAL_INT(run_callback.function, create_rmcios);
                TEST_ASSERT_EQUAL_INT(run_callback.paramtype, binary_rmcios);
                TEST_ASSERT_EQUAL_INT(run_callback.num_params, 2);

                TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].length, sizeof(class_rmcios) );
                TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].required_size, sizeof(class_rmcios) );
                TEST_ASSERT_EQUAL_INT(class_address, *((int *)run_callback.param.bv[0].data) );

                TEST_ASSERT_EQUAL_INT(run_callback.param.bv[1].length, sizeof(void *) );
                TEST_ASSERT_EQUAL_INT(run_callback.param.bv[1].required_size, sizeof(void *) );
                TEST_ASSERT_EQUAL_INT(data_address, *((int *)run_callback.param.bv[1].data) );

                TEST_ASSERT_EQUAL(run_callback.returnv->paramtype, int_rmcios);
                TEST_ASSERT_EQUAL(run_callback.returnv->num_params, 1);

                // Return channel id of created channel:
                *(run_callback.returnv->param.iv) = new_channel_id;
                return;
            }
            int channel_id = create_channel (&context_mock, 0, 0, class_address, data_address);
            TEST_ASSERT_EQUAL_INT(channel_id, new_channel_id); 
        }

        TEST_CASE("name", "Create channel with name")
        {
            static int new_channel_id = 66;
            static const char *name = "john";
            static int class_address = 45;
            static int data_address = 46;

            TEST_CALLBACK(run_callback)
            {   

                switch (run_callback.test_call_index)
                {
                    case 0:
                        // Runs context.create to create the channel 
                        TEST_ASSERT_EQUAL(run_callback.id, context_mock.create);
                        TEST_ASSERT_EQUAL(run_callback.function, create_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.paramtype, binary_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.num_params, 2);

                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].length, sizeof(class_rmcios) );
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].required_size, sizeof(class_rmcios) );
                        TEST_ASSERT_EQUAL_INT(class_address, *((int *)run_callback.param.bv[0].data) );

                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[1].length, sizeof(void *) );
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[1].required_size, sizeof(void *) );
                        TEST_ASSERT_EQUAL_INT(data_address, *((int *)run_callback.param.bv[1].data) );

                        TEST_ASSERT_EQUAL(run_callback.returnv->paramtype, int_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.returnv->num_params, 1);

                        // Return channel id of created channel:
                        *(run_callback.returnv->param.iv) = new_channel_id;
                        break;

                    case 1:
                        // Runs context.name to add name for the channel
                        TEST_ASSERT_EQUAL(run_callback.id, context_mock.name);
                        TEST_ASSERT_EQUAL(run_callback.function, write_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.num_params, 2);
                        TEST_ASSERT_EQUAL(run_callback.paramtype, combo_rmcios);

                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[0].num_params, 1);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[0].paramtype, int_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[0].param.iv[0], 66);

                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].paramtype, buffer_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].num_params, 1);
                        TEST_ASSERT_EQUAL_STR(run_callback.param.cv[1].param.bv[0].data, name);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].length, 4);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].size, 0);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].required_size, 4);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].trailing_size, 0);
                        break;
                }
                return;
            }
            int channel_id = create_channel (&context_mock, name, strlen(name), class_address, data_address);
            TEST_ASSERT_EQUAL_INT(channel_id, new_channel_id); 
        }

        TEST_CASE("param", "Create channel. Name given as parameter")
        {
            static union param_rmcios param = {
                .bv = 0
            };
            static int name_len = 5;
            static char name_mem[5+1];
            static int new_channel_id = 66;
            static const char *name = "name!";
            static int class_address = 45;
            static int data_address = 46;

            TEST_CALLBACK(run_callback)
            {
                switch (run_callback.test_call_index)
                {
                    case 0:
                        // from: param_string_length()
                        TEST_ASSERT_EQUAL_INT(run_callback.id, context_mock.convert);
                        TEST_ASSERT_EQUAL_INT(run_callback.function, read_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.paramtype, buffer_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.num_params, 3);
                        TEST_ASSERT_EQUAL_INT(run_callback.returnv->paramtype, buffer_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.returnv->num_params, 1);
                        run_callback.returnv->param.bv[0].required_size = name_len;
                        break;

                    case 1:
                        // from: allocate_storage()
                        TEST_ASSERT_EQUAL_INT(run_callback.id, context_mock.mem);
                        TEST_ASSERT_EQUAL_INT(run_callback.function, write_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.paramtype, binary_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].length, sizeof(void *));
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].size, 0);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].required_size, sizeof(void *));
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].trailing_size, 0);

                        TEST_ASSERT_EQUAL_INT(run_callback.returnv->paramtype, buffer_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.returnv->num_params, 1);
                        
                        *((void **)run_callback.returnv->param.bv[0].data) = name_mem;
                        break;

                    case 2:
                        // from: param_to_string - copy name data
                        TEST_ASSERT_EQUAL_INT(run_callback.id, context_mock.convert);
                        TEST_ASSERT_EQUAL_INT(run_callback.function, write_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.paramtype, buffer_rmcios);
    
                        TEST_ASSERT_EQUAL_INT(run_callback.returnv->paramtype, buffer_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.returnv->num_params, 1);
                        TEST_ASSERT_EQUAL_INT(run_callback.returnv->param.bv[0].size, name_len + 1);
                        TEST_ASSERT_EQUAL_INT(run_callback.returnv->param.bv[0].data, name_mem);

                        run_callback.returnv->param.bv[0].length = name_len;
                        run_callback.returnv->param.bv[0].required_size = name_len;
                        run_callback.returnv->param.bv[0].trailing_size = 0;
                        run_callback.returnv->param.bv[0].data[0] = 'n';
                        run_callback.returnv->param.bv[0].data[1] = 'a';
                        run_callback.returnv->param.bv[0].data[2] = 'm';
                        run_callback.returnv->param.bv[0].data[3] = 'e';
                        run_callback.returnv->param.bv[0].data[4] = '!';
                        run_callback.returnv->param.bv[0].data[5] = 0;

                        TEST_ASSERT_EQUAL_STR(name_mem, "name!") ;
                        break;

                    case 3:
                        // from: param_to_string() - get length
                        // Don't care
                        break;
                    case 4:
                        // from: create_channel()
                        // Runs context.create to create the channel 
                        TEST_ASSERT_EQUAL(run_callback.id, context_mock.create);
                        TEST_ASSERT_EQUAL(run_callback.function, create_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.paramtype, binary_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.num_params, 2);

                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].length, sizeof(class_rmcios) );
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].required_size, sizeof(class_rmcios) );
                        TEST_ASSERT_EQUAL_INT(class_address, *((int *)run_callback.param.bv[0].data) );

                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[1].length, sizeof(void *) );
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[1].required_size, sizeof(void *) );
                        TEST_ASSERT_EQUAL_INT(data_address, *((int *)run_callback.param.bv[1].data) );

                        TEST_ASSERT_EQUAL(run_callback.returnv->paramtype, int_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.returnv->num_params, 1);

                        // Return channel id of created channel:
                        *(run_callback.returnv->param.iv) = new_channel_id;
                        break;

                    case 5:
                        // Runs context.name to add name for the channel
                        TEST_ASSERT_EQUAL(run_callback.id, context_mock.name);
                        TEST_ASSERT_EQUAL(run_callback.function, write_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.num_params, 2);
                        TEST_ASSERT_EQUAL(run_callback.paramtype, combo_rmcios);

                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[0].num_params, 1);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[0].paramtype, int_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[0].param.iv[0], 66);

                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].paramtype, buffer_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].num_params, 1);
                        TEST_ASSERT_EQUAL_STR(run_callback.param.cv[1].param.bv[0].data, name);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].length, 5);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].size, 0);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].required_size, 5);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].trailing_size, 0);
                        break;
                }
                return;
            }   

            int channel_id = create_channel_param (&context_mock, buffer_rmcios, param, 2, class_address, data_address);
            TEST_ASSERT_EQUAL_INT(channel_id, new_channel_id); 
        }

        TEST_CASE("str", "Create channel. Name given as string")
        {
            static const char *name = "strname";
            static int class_address = 45;
            static int data_address = 46;
            static int new_channel_id = 77;
            
            TEST_CALLBACK(run_callback)
            {   
                switch (run_callback.test_call_index)
                {
                    case 0:
                        // Runs context.create to create the channel 
                        TEST_ASSERT_EQUAL(run_callback.id, context_mock.create);
                        TEST_ASSERT_EQUAL(run_callback.function, create_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.paramtype, binary_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.num_params, 2);

                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].length, sizeof(class_rmcios) );
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].required_size, sizeof(class_rmcios) );
                        TEST_ASSERT_EQUAL_INT(class_address, *((int *)run_callback.param.bv[0].data) );

                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[1].length, sizeof(void *) );
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[1].required_size, sizeof(void *) );
                        TEST_ASSERT_EQUAL_INT(data_address, *((int *)run_callback.param.bv[1].data) );

                        TEST_ASSERT_EQUAL(run_callback.returnv->paramtype, int_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.returnv->num_params, 1);

                        // Return channel id of created channel:
                        *(run_callback.returnv->param.iv) = new_channel_id;
                        break;

                    case 1:
                        // Runs context.name to add name for the channel
                        TEST_ASSERT_EQUAL(run_callback.id, context_mock.name);
                        TEST_ASSERT_EQUAL(run_callback.function, write_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.num_params, 2);
                        TEST_ASSERT_EQUAL(run_callback.paramtype, combo_rmcios);

                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[0].num_params, 1);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[0].paramtype, int_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[0].param.iv[0], new_channel_id);

                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].paramtype, buffer_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].num_params, 1);
                        TEST_ASSERT_EQUAL_STR(run_callback.param.cv[1].param.bv[0].data, name);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].length, strlen(name));
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].size, 0);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].required_size, strlen(name));
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].trailing_size, 0);
                        break;
                }
                return;
            }

            int channel_id = create_channel_str (&context_mock, name, class_address, data_address);
            TEST_ASSERT_EQUAL_INT(channel_id, new_channel_id); 
        }
    }
    
    TEST_SUITE("create_subchannel")
    {
        SUITE_SETUP()

        TEST_CASE("str", "")
        {
            const char * base_name = "base";
            const char * subchannel_suffix = ".subchannel";
            static int base_channel_id = 5555;
            static int class_address = 45;
            static int data_address = 99;
            
            static char name_mem[5+11+1]; // srlen(base_name) + strlen(subchannel_suffix) + 1
            static int new_channel_id = 102;

            TEST_CALLBACK(run_callback)
            {
                switch (run_callback.test_call_index)
                {
                    case 0:
                        // From channel_name() (get length)
                    case 2:
                        // From channel_name() (get name)
                        TEST_ASSERT_EQUAL_INT(run_callback.id, context_mock.name);
                        TEST_ASSERT_EQUAL_INT(run_callback.function, read_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.paramtype, int_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.num_params, 1);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.iv[0], base_channel_id);
 
                        TEST_ASSERT_EQUAL_INT(run_callback.returnv->paramtype, buffer_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.returnv->num_params, 1);

                        if (run_callback.test_call_index == 0) {
                            TEST_ASSERT_EQUAL_INT(run_callback.returnv->param.bv[0].size, 0);
                            TEST_ASSERT_EQUAL_INT(run_callback.returnv->param.bv[0].data, 0);
                            TEST_ASSERT_EQUAL_INT(run_callback.returnv->param.bv[0].length, 0);
                            TEST_ASSERT_EQUAL_INT(run_callback.returnv->param.bv[0].trailing_size, 0);
                        }
                        else if (run_callback.test_call_index == 1) {
                            TEST_ASSERT_EQUAL_INT(run_callback.returnv->param.bv[0].size, strlen(base_name) +strlen(subchannel_suffix) + 1);
                            TEST_ASSERT_EQUAL_INT(run_callback.returnv->param.bv[0].length, 0);
                            TEST_ASSERT_EQUAL_INT(run_callback.returnv->param.bv[0].trailing_size, 0);

                            strcpy(run_callback.returnv->param.bv[0].data, base_name);
                        }

                        run_callback.returnv->param.bv[0].required_size = strlen(base_name);
                        break;
                    case 1:
                        // From allocate_storage (for new name)
                        TEST_ASSERT_EQUAL_INT(run_callback.id, context_mock.mem);
                        TEST_ASSERT_EQUAL_INT(run_callback.function, write_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.paramtype, binary_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].length, sizeof(void *));
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].size, 0);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].required_size, sizeof(void *));
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].trailing_size, 0);

                        TEST_ASSERT_EQUAL_INT(run_callback.returnv->paramtype, buffer_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.returnv->num_params, 1);
                        
                        *((void **)run_callback.returnv->param.bv[0].data) = name_mem;
                        break;

                    case 3:
                        // Runs context.create to create the channel 
                        TEST_ASSERT_EQUAL(run_callback.id, context_mock.create);
                        TEST_ASSERT_EQUAL(run_callback.function, create_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.paramtype, binary_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.num_params, 2);

                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].length, sizeof(class_rmcios) );
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[0].required_size, sizeof(class_rmcios) );
                        TEST_ASSERT_EQUAL_INT(class_address, *((int *)run_callback.param.bv[0].data) );

                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[1].length, sizeof(void *) );
                        TEST_ASSERT_EQUAL_INT(run_callback.param.bv[1].required_size, sizeof(void *) );
                        TEST_ASSERT_EQUAL_INT(data_address, *((int *)run_callback.param.bv[1].data) );

                        TEST_ASSERT_EQUAL(run_callback.returnv->paramtype, int_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.returnv->num_params, 1);

                        // Return channel id of created channel:
                        *(run_callback.returnv->param.iv) = new_channel_id;
                        break;

                    case 4:
                        // Runs context.name to add name for the channel
                        TEST_ASSERT_EQUAL(run_callback.id, context_mock.name);
                        TEST_ASSERT_EQUAL(run_callback.function, write_rmcios);
                        TEST_ASSERT_EQUAL(run_callback.num_params, 2);
                        TEST_ASSERT_EQUAL(run_callback.paramtype, combo_rmcios);

                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[0].num_params, 1);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[0].paramtype, int_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[0].param.iv[0], new_channel_id);

                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].paramtype, buffer_rmcios);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].num_params, 1);
                        TEST_ASSERT_EQUAL_STR(run_callback.param.cv[1].param.bv[0].data, "base.subchannel");
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].length, strlen("base.subchannel"));
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].size, 0);
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].required_size, strlen("base.subchannel"));
                        TEST_ASSERT_EQUAL_INT(run_callback.param.cv[1].param.bv[0].trailing_size, 0);
                        break;
                
                }
                return;
            }
            
            int channel_id = create_subchannel_str (&context_mock, base_channel_id, ".subchannel", class_address, data_address);
            TEST_ASSERT_EQUAL_INT(channel_id, new_channel_id); 
        }
    }
    /* TODO
        TEST_CASE(TEST_RUN_CHANNEL, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_LINK_CHANNEL, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_LINK_CHANNEL_FUNCTION, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_RETURN_INT, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }


        TEST_CASE(TEST_RETURN_FLOAT, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_RETURN_STRING, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_RETURN_BUFFER, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_RETURN_BINARY, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_RETURN_VOID, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_PARAM_TO_INTERGER, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_PARAM_TO_FLOAT, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_PARAM_TO_STRING,"")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_PARAM_TO_BUFFER,"")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_PARAM_TO_BINARY, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_PARAM_TO_CHANNEL, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_PARAM_TO_FUNCTION, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(TEST_PARAM_STRING_LENGTH, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
                int param_string_length (const struct context_rmcios *context,
                        enum type_rmcios paramtype,
                        const union param_rmcios param, int index); 
        }

        TEST_CASE(TEST_PARAM_BUFFER_LENGTH, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
                int param_buffer_length (const struct context_rmcios *context,
                        enum type_rmcios paramtype,
                        const union param_rmcios param, int index);
        }

        TEST_CASE(TEST_PARAM_BINARY_LENGTH, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
                int param_binary_length (const struct context_rmcios *context,
                        enum type_rmcios paramtype,
                        const union param_rmcios param, int index);
        }
        */

    TEST_SUITE("param_string")
    {
        SUITE_SETUP()
        TEST_CASE("alloc_size", "")
        {
            // int param_string_alloc_size (const struct context_rmcios *context,
            //   enum type_rmcios paramtype,
            //   const union param_rmcios param, int index);

            int value = 53;

            // Valid data ptr, but length does not match required_size -> allocate
            TEST_CALLBACK(run_callback)
            {
                static char buffer[1];

                TEST_ASSERT_EQUAL_INT(run_callback.paramtype, int_rmcios);
                TEST_ASSERT_EQUAL_INT(run_callback.id, context_mock.convert);
                TEST_ASSERT_EQUAL_INT(run_callback.function, read_rmcios);
                TEST_ASSERT_EQUAL_INT(run_callback.num_params, 1);
                TEST_ASSERT_EQUAL_INT(run_callback.returnv->paramtype, buffer_rmcios);
                TEST_ASSERT_EQUAL_INT(run_callback.returnv->num_params, 1);

                run_callback.returnv->param.bv[0].data = buffer;
                run_callback.returnv->param.bv[0].length = 1;
                run_callback.returnv->param.bv[0].size = 2;
                run_callback.returnv->param.bv[0].required_size = 2; 
                run_callback.returnv->param.bv[0].trailing_size = 0;
                return;
            }   
            int size = param_string_alloc_size (&context_mock, int_rmcios, (const union param_rmcios)&value, 0);
            TEST_ASSERT_EQUAL_INT(size, 3);     


            // Valid data ptr and Length is matching required size, but no trailing size -> allocate
            TEST_CALLBACK(run_callback)
            {
                static char buffer[2];

                TEST_ASSERT_EQUAL_INT(run_callback.paramtype, int_rmcios);
                TEST_ASSERT_EQUAL_INT(run_callback.id, context_mock.convert);
                TEST_ASSERT_EQUAL_INT(run_callback.function, read_rmcios);
                TEST_ASSERT_EQUAL_INT(run_callback.num_params, 1);
                TEST_ASSERT_EQUAL_INT(run_callback.returnv->paramtype, buffer_rmcios);
                TEST_ASSERT_EQUAL_INT(run_callback.returnv->num_params, 1);

                run_callback.returnv->param.bv[0].data = buffer;
                run_callback.returnv->param.bv[0].length = 2;
                run_callback.returnv->param.bv[0].size = 0;
                run_callback.returnv->param.bv[0].required_size = 2; 
                run_callback.returnv->param.bv[0].trailing_size = 0;
                return;
            }   
            size = param_string_alloc_size (&context_mock, int_rmcios,(const union param_rmcios)&value, 0);
            TEST_ASSERT_EQUAL_INT(size, 3)


                // Valid data ptr and size is big enough is matching and trailing zero -> don't allocate
            TEST_CALLBACK(run_callback)
            {
                static char buffer[3];
                run_callback.returnv->param.bv[0].data = buffer;
                run_callback.returnv->param.bv[0].length = 2;
                run_callback.returnv->param.bv[0].size = 0;
                run_callback.returnv->param.bv[0].required_size = 2; 
                run_callback.returnv->param.bv[0].trailing_size = 1;
                return;
            }   
            size = param_string_alloc_size (&context_mock, int_rmcios,(const union param_rmcios)&value, 0);
            TEST_ASSERT_EQUAL_INT(size, 0)  

            // Trailing byte not zero -> allocate
            TEST_CALLBACK(run_callback)
            {
                static char buffer[3] = {'1','2','F'};
                run_callback.returnv->param.bv[0].data = buffer;
                run_callback.returnv->param.bv[0].length = 2;
                run_callback.returnv->param.bv[0].size = 0;
                run_callback.returnv->param.bv[0].required_size = 2; 
                run_callback.returnv->param.bv[0].trailing_size = 1;
                return;
            }   
            size = param_string_alloc_size (&context_mock, int_rmcios,(const union param_rmcios)&value, 0);
            TEST_ASSERT_EQUAL_INT(size, 3)  

            // Null pointer in data -> allocate
            TEST_CALLBACK(run_callback)
            {
                run_callback.returnv->param.bv[0].data = 0;
                run_callback.returnv->param.bv[0].length = 2;
                run_callback.returnv->param.bv[0].size = 0;
                run_callback.returnv->param.bv[0].required_size = 2; 
                run_callback.returnv->param.bv[0].trailing_size = 1;
                return;
            }
            size = param_string_alloc_size (&context_mock, int_rmcios,(const union param_rmcios)&value, 0);
            TEST_ASSERT_EQUAL_INT(size, 3)       
        }
    }
    TEST_SUITE("param_buffer")
    {
        SUITE_SETUP()
        TEST_CASE("alloc_size", "")
        {
            int value = 53;

            // Valid data ptr and length does not match -> allocate
            TEST_CALLBACK(run_callback)
            {
                static char data[1];

                TEST_ASSERT_EQUAL_INT(run_callback.paramtype, int_rmcios);
                TEST_ASSERT_EQUAL_INT(run_callback.id, context_mock.convert);
                TEST_ASSERT_EQUAL_INT(run_callback.function, read_rmcios);
                TEST_ASSERT_EQUAL_INT(run_callback.num_params, 1);
                TEST_ASSERT_EQUAL_INT(run_callback.returnv->paramtype, buffer_rmcios);
                TEST_ASSERT_EQUAL_INT(run_callback.returnv->num_params, 1);

                run_callback.returnv->param.bv[0].data = data;
                run_callback.returnv->param.bv[0].length = 1;
                run_callback.returnv->param.bv[0].size = 0;
                run_callback.returnv->param.bv[0].required_size = 2; 
                run_callback.returnv->param.bv[0].trailing_size = 0;
                return;
            }
            int size = param_buffer_alloc_size (&context_mock, int_rmcios,(const union param_rmcios)&value, 0);
            TEST_ASSERT_EQUAL_INT(size, 2)     

                // Valid data ptr and length is matching required_size -> no need to allocate
            TEST_CALLBACK(run_callback)
            {
                static char data[2];

                run_callback.returnv->param.bv[0].data = data;
                run_callback.returnv->param.bv[0].length = 2;
                run_callback.returnv->param.bv[0].size = 0;
                run_callback.returnv->param.bv[0].required_size = 2; 
                run_callback.returnv->param.bv[0].trailing_size = 0;
                return;
            }
            size = param_buffer_alloc_size (&context_mock, int_rmcios,(const union param_rmcios)&value, 0);
            TEST_ASSERT_EQUAL_INT(size, 0)       

            // Null pointer in data -> allocate
            TEST_CALLBACK(run_callback)
            {
                run_callback.returnv->param.bv[0].data = 0;
                run_callback.returnv->param.bv[0].length = 2;
                run_callback.returnv->param.bv[0].size = 0;
                run_callback.returnv->param.bv[0].required_size = 2; 
                run_callback.returnv->param.bv[0].trailing_size = 0;
                return;
            }
            size = param_buffer_alloc_size (&context_mock, int_rmcios,(const union param_rmcios)&value, 0);
            TEST_ASSERT_EQUAL_INT(size, 2)       
        }
    }
        /* TODO

           TEST_CASE(TEST_PARAM_BUFFER_ALLOC_SIZE_0, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(READ_F, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(READ_I, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(READ_STR, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(WRITE_F, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(WRITE_FV, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(WRITE_I, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(WRITE_IV, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(WRITE_STR, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(WRITE_BUFFER, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(WRITE_BINARY, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(LINKED_CHANNELS, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(ALLOCATE_STORAGE, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

           TEST_CASE(FREE_STORAGE, "")
           {
           TEST_ASSERT_EQUAL_INT(1, 0)       
           }

        TEST_CASE(FUNCTION_ENUM, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(CHANNEL_ENUM, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(CHANNEL_NAME, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }

        TEST_CASE(PARAM_TO_INT, "")
        {
            TEST_ASSERT_EQUAL_INT(1, 0)       
        }
    }*/
}

int main(void)
{
    TEST_RUN(test_data)
    return TEST_RESULTS(test_data)
}

