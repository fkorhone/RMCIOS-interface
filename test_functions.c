#include <stdio.h>
#include <string.h>

#include "test.h"
#include "RMCIOS-API.h"
#include "RMCIOS-functions.c"

#define TEST_FUNC_NAME run_stub
#define TEST_CALLBACK_NAME run_callback
#define TEST_FUNC_RETURN_TYPE void
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
        .convert = 55,
        .create = 56,
        .name = 67
};

TEST_RUNNER
{
    TEST_SUITE("MAIN_SUITE")
    {
        default: 

    TEST_CASE("TEST_CREATE_CHANNEL_NO_NAME", "")
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
    
    TEST_CASE("TEST_CREATE_CHANNEL", "")
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
    /*
    TEST_CASE(TEST_CREATE_CHANNEL_PARAM, "")
    {
        TEST_ASSERT_EQUAL_INT(1, 0)       
    }

    TEST_CASE(TEST_CREATE_CHANNEL_STR, "")
    {
        TEST_ASSERT_EQUAL_INT(1, 0)       
    }

    TEST_CASE(TEST_CREATE_SUBCHANNEL_STR, "")
    {
        TEST_ASSERT_EQUAL_INT(1, 0)       
    }
    
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

    TEST_CASE("TEST_PARAM_STRING_ALLOC_SIZE", "")
    {
   /* int param_string_alloc_size (const struct context_rmcios *context,
                                 enum type_rmcios paramtype,
                                 const union param_rmcios param, int index);*/

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

    TEST_CASE("TEST_PARAM_BUFFER_ALLOC_SIZE", "")
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
    /*

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
    }*/
    }
}

int main(void)
{
    TEST_RUN(test_data)
    return TEST_RESULTS(test_data)
}

