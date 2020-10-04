Just a little toy
I wanted to play with recursive descent parser

This parses grammar rules like this 

```
	Sum     -> Sum     [+-] Product | Product
	Product -> Product [*/] Factor | Factor
	Factor  -> '(' Sum ')' | Number
	Number  -> [0-9] Number | [0-9]
```

It also has error reporting
