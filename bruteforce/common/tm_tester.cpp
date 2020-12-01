#include "data_sizes.h"
#include "tm_tester.h"
#include "tm_base.h"

tm_tester::tm_tester(TM_base* obj, key_schedule_entry* entries) : UUT(obj), schedule_entries(entries)
{

}

void tm_tester::process_load_fetch(uint8* data_in, uint8* data_out)
{
	UUT->load_data(data_in);
	UUT->fetch_data(data_out);
}

void tm_tester::run_iterations(uint8 * test_case, uint16 * rng_seed, int algorithm, int iterations)
{
	UUT->load_data(test_case);

	UUT->run_alg(algorithm, rng_seed, iterations);

	UUT->fetch_data(test_case);
}

void tm_tester::run_expansion(uint32 key, uint32 data, uint8 * result_data)
{
	UUT->expand(key, data);

	UUT->fetch_data(result_data);
}

void tm_tester::run_full_process(uint32 key, uint32 data, key_schedule_entry * entries, uint8* result_data)
{
	UUT->expand(key, data);
	UUT->run_all_maps(entries);
	UUT->fetch_data(result_data);
}

void tm_tester::process_alg_test_case(uint8 * test_case, uint16 * rng_seed, int algorithm)
{
	UUT->load_data(test_case);

	UUT->run_alg(algorithm, rng_seed, 1);

	UUT->fetch_data(test_case);
}

void tm_tester::run_full_process(uint32 key, uint32 data)
{
	UUT->expand(key, data);
	UUT->run_all_maps(schedule_entries);
}