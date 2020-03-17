#pragma once

#include "../Includes/JSON.h"
#include "../Buffers.h"
#include "../../../string_ops/include/string_ops.h"

namespace gamelib
{
	template <typename BUFFER, typename ROW_CALLBACK>
	intptr_t LoadCSV(BUFFER& buffer, ROW_CALLBACK&& row_callback)
	{
		bool in_quote = false;
		intptr_t line = 0;

		std::vector<std::string> row;
		std::string current_cell;
		char cp = 0;
		while (buffer_read_to(buffer, cp))
		{
			if (in_quote)
			{
				if (cp == '"')
				{
					if (!buffer_read_to(buffer, cp))
						break;
					if (cp != '"')
					{
						in_quote = false;
						goto no_quote;
					}
				}
				current_cell += cp;
			}
			else
			{
			no_quote:
				
				if (cp == '\r')
				{
					if (!buffer_read_to(buffer, cp))
						break;
					if (cp != '\n')
					{
						current_cell += '\r';
						current_cell += cp;
						continue;
					}
				}
				
				if (cp == '\n')
				{
					row.push_back(std::move(current_cell));
					row_callback(line++, std::move(row));
				}
				else if (cp == '"')
					in_quote = true;
				else if (cp == ',')
					row.push_back(std::move(current_cell));
				else
					current_cell += cp;
			}
		}

		if (!current_cell.empty())
			row.push_back(std::move(current_cell));
		if (!row.empty())
			row_callback(line++, std::move(row));

		return line;
	}

	template <typename BUFFER>
	json LoadCSV(BUFFER& buffer)
	{
		json result = json::array_t{};

		std::vector<std::string> column_names;
		LoadCSV(buffer, [&](intptr_t line, std::vector<std::string> row) {
			if (line == 0)
				column_names = std::move(row);
			else
			{
				auto& obj = result.emplace_back();
				for (size_t i = 0; i < row.size() && i < column_names.size(); i++)
				{
					auto& cell = row[i];
					/*
					if (!cell.empty() && cell[0] == '-' || string_ops::isdigit(cell[0]))
					{
						if (cell.find('.') != std::string::npos)
							obj[column_names[i]] = std::stod(cell);
						else
							obj[column_names[i]] = std::stoll(cell);
					}
					else
						obj[column_names[i]] = cell;
						*/
					auto result = json::parse(cell, nullptr, false);
					if (result.type() == json::value_t::discarded)
						obj[column_names[i]] = std::move(cell);
					else
						obj[column_names[i]] = std::move(result);
				}
			}
		});

		return result;
	}
}