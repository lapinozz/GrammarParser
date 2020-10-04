#pragma once

#include <variant>
#include <string>
#include <vector>

struct Rule
{
	using NonTerminalSymbol = std::wstring;

	using LiteralTerminalSymbol = wchar_t;

	struct RangeTerminalSymbol
	{
		LiteralTerminalSymbol start;
		LiteralTerminalSymbol end;
	};

	using ChoiceTerminalSymbol = std::vector<std::variant<LiteralTerminalSymbol, RangeTerminalSymbol>>;

	using TerminalSymbol = std::variant<LiteralTerminalSymbol, ChoiceTerminalSymbol>;

	using Symbol = std::variant<TerminalSymbol, NonTerminalSymbol>;

	std::wstring name;
	std::vector<Symbol> symbols;
};

using Grammar = std::vector<Rule>;