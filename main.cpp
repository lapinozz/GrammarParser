#include <iomanip>
#include <iostream>

#include "parser.hpp"
#include "lexer.hpp"

int main()
{
	const std::wstring input = LR"=(
	Sum     -> Sum     [+-] Product | Product
	Product -> Product [*/] Factor | Factor
	Factor  -> '(' Sum ')' | Number
	Number  -> [0-9] Number | [0-9]
	)=";

	std::wstringstream sstr(input);
	Lexer lexer(sstr);

	Grammar grammar;

	Parser parser(lexer, grammar);

	if (!parser.parseGrammar())
	{
		const auto error = parser.getError();

		std::wcout << "Error: " << error.message << std::endl;
		std::wcout << "Line: " << error.state.line << "(" << error.state.column << ")" << std::endl;

		sstr.seekg(error.state.offset - error.state.column, std::ios_base::beg);
		std::wstring line;
		std::getline(sstr, line);
		std::wcout << line << std::endl;
		std::wcout << std::wstring(error.state.column, ' ') << '^' << std::endl;
		
		std::cin.get();
	}

	std::wcout << "---------------------------------" << std::endl;

	for (const auto& rule : grammar)
	{
		std::wcout << rule.name << L" -> ";
		for (const auto& s : rule.symbols)
		{
			std::visit([](auto&& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				if constexpr (std::is_same_v<T, Rule::NonTerminalSymbol>)
				{
					std::wcout << arg << ' ';
				}
				else if constexpr (std::is_same_v<T, Rule::TerminalSymbol>)
				{
					std::visit([](auto&& arg)
					{
						using T = std::decay_t<decltype(arg)>;
						if constexpr (std::is_same_v<T, Rule::LiteralTerminalSymbol>)
						{
							std::wcout << '"' << arg << L"\" ";
						}
						else if constexpr (std::is_same_v<T, Rule::ChoiceTerminalSymbol>)
						{
							std::wcout << '[';
							for (const auto& s : arg)
							{
								std::visit([](auto&& arg)
								{
									using T = std::decay_t<decltype(arg)>;
									if constexpr (std::is_same_v<T, Rule::LiteralTerminalSymbol>)
									{
										std::wcout << arg;
									}
									else if constexpr (std::is_same_v<T, Rule::RangeTerminalSymbol>)
									{
										std::wcout << '(' << arg.start << " to " << arg.end << ')';
									}
								}, s);
							}
							std::wcout << L"] ";
						}
					}, arg);
				}
			}, s);
		}
		std::wcout << std::endl;
	}
	std::cin.get();
}