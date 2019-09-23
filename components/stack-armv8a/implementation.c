/*| headers |*/
#include <stdint.h>
#include <stddef.h>

/*| object_like_macros |*/

/*| types |*/

/*| structures |*/

/*| extern_declarations |*/

/*| function_declarations |*/

/*| state |*/
{{#tasks}}
static uint64_t stack_{{idx}}[{{stack_size}}] __attribute__((aligned(16)));
{{/tasks}}

/*| function_like_macros |*/

/*| functions |*/

/*| public_functions |*/

/*| public_privileged_functions |*/
