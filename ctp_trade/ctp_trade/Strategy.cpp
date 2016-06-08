#include "Strategy.h"

#include <string>
#include <algorithm>

#include "PublicDataStruct.h"
#include "EfficientMap.h"

bool CStrategy::initial_stg(std::string config_path, std::string config_head, int k_type)
{
	bool init_ret = true;

	// Read Interested Instruments From Config File
	init_ret &= load_focused_inst(config_path, config_head, k_type);
	
	save_min_data.set_root_dir(config_path.substr(0, config_path.find_last_of("\\")));
	init_ret &= save_min_data.open_save_data(config_head);

	// Other Operation
	// .

	return init_ret;
}

void  CStrategy::release_stg()
{
	save_min_data.close_save_data();
}

void CStrategy::update(int period, candle_bar& stg_data)
{
	// if the strategy is interested in 'period'
	if (decision_data_.find(period) != decision_data_.end())
	{
		std::string ins = stg_data.bar_name;
		// if the strategy which based on 'period' that focuses on the 'ins'
		if (decision_data_[period].find(ins) != decision_data_[period].end())
		{
			archive_data aha_data;
			aha_data.period = period;
			memcpy_s(&aha_data.data, sizeof(candle_bar), &stg_data, sizeof(candle_bar));

			save_min_data.write_save_data(aha_data);
			decision_data_[period][ins].push_bar(stg_data);
		}
	}
}

bool CStrategy::load_focused_inst(std::string& config_path, std::string& config_head, int k_type)
{
	bool cfg_ret = false;

	std::ifstream config_file(config_path);

	if (!config_file.is_open())
	{
		return cfg_ret;
	}

	std::string string_line;
	while (getline(config_file, string_line))
	{
		if (std::string::npos != string_line.find(config_head))
		{
			if (std::string::npos != string_line.find_first_of("="))
			{
				// Trim Space
				string_line.erase(std::remove_if(string_line.begin(), string_line.end(), [](char c){return c == ' '; }), string_line.end());

				// Position Instruments
				size_t ins_pos = string_line.find_first_of("=");

				size_t beg_pos = ins_pos + 1;
				ins_pos = string_line.find_first_of("|", beg_pos);
				while (std::string::npos != ins_pos)
				{
					construct_decision_data(string_line.substr(beg_pos, ins_pos - beg_pos), k_type);
					
					beg_pos = ins_pos + 1;
					ins_pos = string_line.find_first_of("|", beg_pos);
				}
				if (ins_pos > beg_pos)
				{
					construct_decision_data(string_line.substr(beg_pos, ins_pos - beg_pos), k_type);
				}

				cfg_ret = true;
				break;
			}
			else
			{
				break;
			}
		}
		else
		{
			continue;
		}
	}

	return cfg_ret;
}

void CStrategy::construct_decision_data(std::string ins, int k_type)
{
	if (k_type & CCandleBar::MIN_ONE)
	{
		CCandleBar k_data;
		k_data.set_candle_type(CCandleBar::MIN_ONE);

		std::map<std::string, CCandleBar> instr_kdata;
		if (!decision_data_[CCandleBar::MIN_ONE].empty())
		{
			instr_kdata = decision_data_[CCandleBar::MIN_ONE];
		}

		efficient_map_operation(instr_kdata, ins, k_data);
		efficient_map_operation(decision_data_, CCandleBar::MIN_ONE, instr_kdata);
	}

	if (k_type & CCandleBar::MIN_FIVE)
	{
		CCandleBar k_data;
		k_data.set_candle_type(CCandleBar::MIN_FIVE);

		std::map<std::string, CCandleBar> instr_kdata;
		if (!decision_data_[CCandleBar::MIN_FIVE].empty())
		{
			instr_kdata = decision_data_[CCandleBar::MIN_FIVE];
		}

		efficient_map_operation(instr_kdata, ins, k_data);
		efficient_map_operation(decision_data_, CCandleBar::MIN_FIVE, instr_kdata);
	}

	if (k_type & CCandleBar::MIN_FIVETEEN)
	{
		CCandleBar k_data;
		k_data.set_candle_type(CCandleBar::MIN_FIVETEEN);

		std::map<std::string, CCandleBar> instr_kdata;
		if (!decision_data_[CCandleBar::MIN_FIVETEEN].empty())
		{
			instr_kdata = decision_data_[CCandleBar::MIN_FIVETEEN];
		}

		efficient_map_operation(instr_kdata, ins, k_data);
		efficient_map_operation(decision_data_, CCandleBar::MIN_FIVETEEN, instr_kdata);
	}

	if (k_type & CCandleBar::MIN_THIRTY)
	{
		CCandleBar k_data;
		k_data.set_candle_type(CCandleBar::MIN_THIRTY);

		std::map<std::string, CCandleBar> instr_kdata;
		if (!decision_data_[CCandleBar::MIN_THIRTY].empty())
		{
			instr_kdata = decision_data_[CCandleBar::MIN_THIRTY];
		}

		efficient_map_operation(instr_kdata, ins, k_data);
		efficient_map_operation(decision_data_, CCandleBar::MIN_THIRTY, instr_kdata);
	}

	if (k_type & CCandleBar::MIN_SIXTY)
	{
		CCandleBar k_data;
		k_data.set_candle_type(CCandleBar::MIN_SIXTY);

		std::map<std::string, CCandleBar> instr_kdata;
		if (!decision_data_[CCandleBar::MIN_SIXTY].empty())
		{
			instr_kdata = decision_data_[CCandleBar::MIN_SIXTY];
		}

		efficient_map_operation(instr_kdata, ins, k_data);
		efficient_map_operation(decision_data_, CCandleBar::MIN_SIXTY, instr_kdata);
	}
}

//////////////////////////////////////////////////////////////////////////
void CMAStrategy::calculate_indicator_function(void* data)
{
	CppThread*		handle_th = static_cast<CppThread*>(data);
	CMAStrategy*	handle_ma = static_cast<CMAStrategy*>(handle_th->get_data());

	std::map<int, std::map<std::string, int>>&				ma_indicator_pos = handle_ma->calculate_position_flag_;
	std::map<int, std::map<std::string, CMovingAverage>>&	ma_indicator_set = handle_ma->decision_tech_ma_;
	while (!handle_th->is_stop())
	{
		// Calculate Golden/Dead Fork Or Upward/Downward Trend
		for_each(ma_indicator_set.begin(), ma_indicator_set.end(), [handle_ma, &ma_indicator_pos](std::map<int, std::map<std::string, CMovingAverage>>::reference& ma_node)
		{
			int ma_type = ma_node.first;		// K's Type

			std::map<std::string, int>&				ins_pos = ma_indicator_pos[ma_type];
			std::map<std::string, CMovingAverage>&	ins_ma	= ma_node.second;

			for_each(ins_ma.begin(), ins_ma.end(), [handle_ma, &ins_pos](std::map<std::string, CMovingAverage>::reference& node)
			{
				std::string inst = node.first;	// Instrument's Name

				int& pos			= ins_pos[inst];
				CMovingAverage& ma	= node.second;

				// Indicator Calculation
				int action_reason = TECH_REASON_NOTHING;
				int action = ma.get_signal(pos++, CCandleBar::CLOSE, action_reason);
				// Trigger Action
				handle_ma->trigger_signal_action(action, action_reason);
			});
		});
	}
}

bool CMAStrategy::initial_ma_stg(std::string config_path, std::string config_head, int k_type)
{
	// Initial Public Part Of Strategy
	initial_stg(config_path, config_head, k_type);

	// Initial Moving Average Indicator On Different KType
	for_each(decision_data_.begin(), decision_data_.end(), [this](std::map<int, std::map<std::string, CCandleBar>>::reference& data_node)
	{
		int candle_type = data_node.first;
		std::map<std::string, CCandleBar>& ins_candles = data_node.second;

		for_each(ins_candles.begin(), ins_candles.end(), [this, &candle_type](std::map<std::string, CCandleBar>::reference& candle_node)
		{
			CMovingAverage ma(candle_node.second);
			ma.set_default_parameters();

			std::map<std::string, CMovingAverage> ma_map;
			if (!decision_tech_ma_[candle_type].empty())
			{
				ma_map = decision_tech_ma_[candle_type];
			}


			efficient_map_operation(ma_map, candle_node.first, ma);
			efficient_map_operation(decision_tech_ma_, candle_type, ma_map);

			//////////////////////////////////////////////////////////////////////////
			int init_pos = 0;
			std::map<std::string, int>	pos_map;
			if (!calculate_position_flag_[candle_type].empty())
			{
				pos_map = calculate_position_flag_[candle_type];
			}

			efficient_map_operation(pos_map, candle_node.first, init_pos);
			efficient_map_operation(calculate_position_flag_, candle_type, pos_map);

		});
	});

	// Open Indicator Calculation Thread
	indicator_thread_.set_data(this);
	indicator_thread_.create_thread(calculate_indicator_function);

	return true;
}

void CMAStrategy::release_ma_stg()
{
	// Release Public Part Of Strategy
	release_stg();

	indicator_thread_.close_thread();
}

void CMAStrategy::trigger_signal_action(int action, int action_reason)
{
	if (SIGNAL_ISBUY(action))
	{
		if (action_reason == TECH_REASON_GOLDFORK)
		{
			// Golden Fork
		}
		else if (action_reason == TECH_REASON_LONG)
		{
			// Upward Trend
		}
		else
		{

		}
	}
	else if (SIGNAL_ISSELL(action))
	{
		if (action_reason == TECH_REASON_DEADFORK)
		{
			// Dead Fork
		}
		else if (action_reason == TECH_REASON_SHORT)
		{
			// Downward Trend
		}
		else
		{

		}
	}
	else
	{

	}
}