#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>
#include <vector>

class Config
{
public:
	Config(int argc, char ** argv);

	/**
	 * @brief argsOf returns the list of option 'id' arguments
	 * @param id
	 * @return the vector of arguments of the 'id' option
	 */
	const std::vector<std::string> & argsOf(const std::string & id) const;

	/**
	 * @brief operator [] is a helper operator to get the first option's argument
	 *				without passing by the vector representation of argsOf()
	 * @param id
	 * @return the first option's argument if any, otherwise an empty string
	 */
	const std::string & operator[](const std::string & id) const;

	/**
	 * @brief has tells the user if an option is present or not
	 * @param id
	 * @return a boolean telling whether of not the option 'id' is present
	 */
	bool has(const std::string & id) const;

	/**
	 * @brief options getter
	 * @return the options std::map
	 */
	const std::map<std::string, std::vector<std::string> > & options() const;
private:
	std::map<std::string, std::vector<std::string> > _options;

	const static std::string nullstring;
};

#endif // CONFIG_H
