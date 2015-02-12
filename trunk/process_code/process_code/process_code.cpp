#include <stdio.h>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

#include "boinc.h"

#include "data_sizes.h"
#include "cmd_args.h"
#include "rng.h"
#include "working_code.h"
#include "key_schedule.h"
#include "verify.h"

int main(int argc, char **argv)
{
	// Usage:
	//  -k, --key "XXXXXXXX": supply 8 hex values to use as the key value, defaults to all 0's?
	//  -d, --data "XXXXXXXX": supply 8 hex values to use as the starting data value, defaults to all 0's
	//  -c, --count 
	//  -a, --attack attack_type:
	//               "individual": each IV is processed and checked individually
	//               "vector":     each working code is added to a std::vector, processed one round, sorted, then std::unique'd
	//               "hash":       each working code is processed a round, then checked against a hash table. collisions are not processed any further
	//  -b, --big-registers: use 128bit registers as an optimization
	//  -s, --key-schedule: pre-process the key schedule as an optimization instead of processing it for every IV
	//  -f, --file filename: read a list of IVs to process from a file. Overrides -k and -d

	int map_list[26] = { 0x00, 0x02, 0x05, 0x04, 0x03, 0x1D, 0x1C, 0x1E, 0x1B, 0x07, 0x08, 0x06, 0x09, 0x0C, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x0E, 0x0F, 0x10, 0x12, 0x11};

	initialize_boinc();

	generate_rng_table();

	// Process command line
	process_command_line(argc, argv);


	uint8 value[8];

	uint64 IVs_to_run = command_line_options.iv_count;

	uint64 IVs_finished = 0;

	std::ifstream myfile;
	if (command_line_options.from_file)
	{
		myfile.open(command_line_options.filename, std::ios::in);
		if (!myfile.is_open())
		{
			boinc_log("File open error\n");

			return 1;
		}
	}

	if (command_line_options.attack == INDIVIDUAL)
	{
		while (true)
		{
			// Get a key
			if (command_line_options.from_file)
			{
				std::string line;
				if (!getline(myfile, line))
				{
					break;
				}

				std::stringstream ss;
				ss << std::hex << line.substr(0,8);

				uint32 key;
				ss >> key;

				std::stringstream ss2;
				ss2 << std::hex << line.substr(8,16);

				uint32 data;
				ss2 >> data;

				// something about skipping bad lines?

				value[0] = (key >> 24) & 0xFF;
				value[1] = (key >> 16) & 0xFF;
				value[2] = (key >> 8) & 0xFF;
				value[3] = key & 0xFF;

				value[4] = (data >> 24) & 0xFF;
				value[5] = (data >> 16) & 0xFF;
				value[6] = (data >> 8) & 0xFF;
				value[7] = data & 0xFF;
			}
			else
			{
				if (IVs_to_run == IVs_finished)
				{
					break;
				}

				if (IVs_finished == 0)
				{
					value[0] = (command_line_options.start_key >> 24) & 0xFF;
					value[1] = (command_line_options.start_key >> 16) & 0xFF;
					value[2] = (command_line_options.start_key >> 8) & 0xFF;
					value[3] = command_line_options.start_key & 0xFF;

					value[4] = (command_line_options.start_data >> 24) & 0xFF;
					value[5] = (command_line_options.start_data >> 16) & 0xFF;
					value[6] = (command_line_options.start_data >> 8) & 0xFF;
					value[7] = command_line_options.start_data & 0xFF;
				}
				else
				{
					// Increment to the next IV
					for (int i = 7; i >= 0; i--)
					{
						value[i]++;
						if (value[i] != 0x00)
						{
							break;
						}
					}
				}
			}

			// Pre-process the key schedule
			key_schedule_data schedule_data;
			schedule_data.as_uint8[0] = value[0];
			schedule_data.as_uint8[1] = value[1];
			schedule_data.as_uint8[2] = value[2];
			schedule_data.as_uint8[3] = value[3];

			key_schedule_entry schedule_entries[27];

			int schedule_counter = 0;
			for (int i = 0; i < 26; i++)
			{
				schedule_entries[schedule_counter++] = generate_schedule_entry(map_list[i],&schedule_data);

				if (map_list[i] == 0x22)
				{
					schedule_entries[schedule_counter++] = generate_schedule_entry(map_list[i],&schedule_data,4);
				}
			}

			working_code in_progress(value);

			schedule_counter = 0;

			for (int map_index = 0; map_index < 26; map_index++)
			{
				in_progress.process_map_exit(map_list[map_index],schedule_entries[schedule_counter]);

				if (map_list[map_index] == 0x22)
				{
					in_progress.process_map_exit(map_list[map_index],schedule_entries[schedule_counter+1]);
				}

				schedule_counter++;
				if (map_list[map_index] == 0x22)
				{
					schedule_counter++;
				}
			}

			output_stats(&in_progress);

			IVs_finished++;

			if (command_line_options.from_file)
			{
				// TODO
			}
			else
			{
				fraction_done(((double)IVs_finished)/((double)IVs_to_run));
			}
		}
	}
	else if (command_line_options.attack == VECTOR)
	{
		boinc_log("vector attack\n");
		// will force a pre-processed key schedule for now

		std::vector<working_code> IVs_in_progress;

		key_schedule_entry schedule_entries[27];

		while (true)
		{
			// Get a key
			if (command_line_options.from_file)
			{
				std::string line;
				if (!getline(myfile, line))
				{
					break;
				}

				std::stringstream ss;
				ss << std::hex << line.substr(0,8);

				uint32 key;
				ss >> key;

				std::stringstream ss2;
				ss2 << std::hex << line.substr(8,16);

				uint32 data;
				ss2 >> data;

				// something about skipping bad lines?

				value[0] = (key >> 24) & 0xFF;
				value[1] = (key >> 16) & 0xFF;
				value[2] = (key >> 8) & 0xFF;
				value[3] = key & 0xFF;

				value[4] = (data >> 24) & 0xFF;
				value[5] = (data >> 16) & 0xFF;
				value[6] = (data >> 8) & 0xFF;
				value[7] = data & 0xFF;
			}
			else
			{
				if (IVs_to_run == IVs_finished)
				{
					break;
				}

				if (IVs_finished == 0)
				{
					value[0] = (command_line_options.start_key >> 24) & 0xFF;
					value[1] = (command_line_options.start_key >> 16) & 0xFF;
					value[2] = (command_line_options.start_key >> 8) & 0xFF;
					value[3] = command_line_options.start_key & 0xFF;

					value[4] = (command_line_options.start_data >> 24) & 0xFF;
					value[5] = (command_line_options.start_data >> 16) & 0xFF;
					value[6] = (command_line_options.start_data >> 8) & 0xFF;
					value[7] = command_line_options.start_data & 0xFF;
				}
				else
				{
					// Increment to the next IV
					for (int i = 7; i >= 0; i--)
					{
						value[i]++;
						if (value[i] != 0x00)
						{
							break;
						}
					}
				}
			}

			// Get the key from the first IV
			if (IVs_finished == 0)
			{
				key_schedule_data schedule_data;
				// Pre-process the key schedule
				schedule_data.as_uint8[0] = value[0];
				schedule_data.as_uint8[1] = value[1];
				schedule_data.as_uint8[2] = value[2];
				schedule_data.as_uint8[3] = value[3];

				int schedule_counter = 0;
				for (int i = 0; i < 26; i++)
				{
					schedule_entries[schedule_counter++] = generate_schedule_entry(map_list[i],&schedule_data);

					if (map_list[i] == 0x22)
					{
						schedule_entries[schedule_counter++] = generate_schedule_entry(map_list[i],&schedule_data,4);
					}
				}
			}

			IVs_in_progress.push_back(value);

			IVs_finished++;
		}

		int schedule_counter = 0;

		for (int map_index = 0; map_index < 26; map_index++)
		{
			uint64 count = 0;
			for (std::vector<working_code>::iterator it = IVs_in_progress.begin(); it != IVs_in_progress.end(); ++it)
			{
				it->process_map_exit(map_list[map_index],schedule_entries[schedule_counter]);

				if (map_list[map_index] == 0x22)
				{
					it->process_map_exit(map_list[map_index],schedule_entries[schedule_counter+1]);
				}

				count++;

				if (command_line_options.from_file)
				{
					// TODO
				}
				else
				{
					// TODO
					fraction_done((((double)count)+((double)(map_index*command_line_options.iv_count)))/(((double)26)*((double)command_line_options.iv_count)));
				} 
			}

			// Advance the schedule
			schedule_counter++;
			if (map_list[map_index] == 0x22)
			{
				schedule_counter++;
			}

			boinc_log("%lu run\n",IVs_in_progress.size());

			size_t x = IVs_in_progress.size();
			std::sort(IVs_in_progress.begin(), IVs_in_progress.end());
			IVs_in_progress.erase(std::unique(IVs_in_progress.begin(),IVs_in_progress.end()),IVs_in_progress.end());
			x -= IVs_in_progress.size();
			boinc_log("%lu deleted\n",x);

			boinc_log("\n");
		}

		boinc_log("%lu checking\n\n",IVs_in_progress.size());

		for (std::vector<working_code>::iterator it = IVs_in_progress.begin(); it != IVs_in_progress.end(); ++it)
		{
			output_stats(&(*it));
		}
	}

	finish_boinc();

	return 0;
}