#include <lib/config.h>

#include <iostream>

const std::string Config::nullstring = std::string();

Config::Config(int argc, char ** argv)
{
	std::string current_opt = "";
	std::vector<std::string> opt_args;

	for (int i = 1; i < argc; ++i)
	{
		char * arg = argv[i];
		if (arg[0] == '-' && current_opt != "-")
		{
			if (current_opt != "")
				_options[current_opt] = opt_args;

			current_opt = arg+1;
			opt_args.clear();
		}
		else if (current_opt != "")
		{
			opt_args.emplace_back(arg);
		}
		else
		{
			throw std::string("bad configuration format");
		}
	}

	if (current_opt != "")
		_options[current_opt] = opt_args;
}

const std::vector<std::string> & Config::argsOf(const std::string & id) const
{
	return _options.at(id);
}

const std::string & Config::operator[](const std::string & id) const
{
	if (!has(id) || _options.at(id).empty())
		return nullstring;
	else
		return _options.at(id).front();
}

bool Config::has(const std::string &id) const
{
	try
	{
		return _options.at(id).size() >= 0;
	}
	catch (...)
	{
		return false;
	}
}

const std::map<std::string, std::vector<std::string> > &Config::options() const
{
	return _options;
}
