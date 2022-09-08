#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>

#include "json.hpp"

static std::unique_ptr<std::string> exec(const std::string& cmd)
{
	auto stream = popen(cmd.c_str(), "r");
	if(!stream)
		return nullptr;
	char buffer[256];

	auto data = std::make_unique<std::string>();
	while (!feof(stream))
		if (fgets(buffer, sizeof(buffer), stream) != NULL)
			data->append(buffer);
	pclose(stream);
	return data;
}

static std::unique_ptr<std::string> git(const std::string& cmd)
{
	auto stdout = exec("git " + cmd);
	while(stdout && stdout->size() && (stdout->at(stdout->size() - 1) == '\r' || stdout->at(stdout->size() - 1) == '\n'))
		stdout->pop_back();
	if(stdout->empty())
		return nullptr;
	return stdout;
}

static std::unique_ptr<size_t> parseSize(const std::string& str)
{
	if(str.empty())
		return nullptr;

	size_t result = 0;
	for(size_t i = 0; i < str.length(); i++)
	{
		auto ch = str[i];
		if(!isdigit(ch))
			return nullptr;
		auto n = ch - '0';
		size_t nextResult = result * 10 + n;
		if(nextResult < result)
			return nullptr;
		result = nextResult;
	}
	return std::make_unique<size_t>(result);
}

int main(int argc, char* argv[])
{
	auto gitName = git("config --local user.name");
	auto gitEmail = git("config --local user.email");

	if(argc > 1 && strcmp(argv[1], "check") == 0)
	{
		if(gitName && gitEmail)
		{
			fprintf(stderr, "%s <%s>\n", gitName->c_str(), gitEmail->c_str());
			return EXIT_SUCCESS;
		}
		else
		{
			fprintf(stderr, "Name or email not set!\n");
			return EXIT_FAILURE;
		}
	}

	std::string homepath = getenv("HOME");
	auto jsonFilename = "git-identity.json";
	std::ifstream jsonFile(homepath + "/" + jsonFilename);
	if(!jsonFile.is_open())
	{
		fprintf(stderr, "Failed to open '%s'!\n", jsonFilename);
		return EXIT_FAILURE;
	}

	auto json = nlohmann::json::parse(jsonFile);

	size_t defaultChoice = 0;
	for(size_t i = 0; i < json.size(); i++)
	{
		auto& identity = json[i];
		std::string name = identity["user.name"];
		std::string email = identity["user.email"];
		if(gitName && gitEmail && name == *gitName && email == *gitEmail)
			defaultChoice = i + 1;
		printf("(%zu) %s <%s>\n", i + 1, name.c_str(), email.c_str());
		for(auto& el : identity.items())
		{
			std::string key = el.key();
			if(key == "user.name" || key == "user.email")
				continue;
			std::string value = el.value();
			printf("    %s", key.c_str());
			for(size_t i = 0; i + key.length() < 20; i++)
				printf(" ");
			printf("%s\n", value.empty() ? "<empty>" : value.c_str());
		}
	}

	if(defaultChoice)
		printf("\nYour choice (%zu): ", defaultChoice);
	else
		printf("\nYour choice: ");

	std::string choiceStr;
	std::getline(std::cin, choiceStr);

	auto choice = (choiceStr.empty() && defaultChoice > 0) ? std::make_unique<size_t>(defaultChoice) : parseSize(choiceStr);
	if(!choice || *choice == 0 || *choice > json.size())
	{
		fprintf(stderr, "Invalid choice '%s'!\n", choiceStr.c_str());
		return EXIT_FAILURE;
	}

	auto identity = json[*choice - 1];
	for(auto& el : identity.items())
	{
		std::string key = el.key();
		std::string value = el.value();
		git("config \"" + key + "\" \"" + value + "\"");
	}

	return EXIT_FAILURE;
}
