# simple-compiler

Implementing a simple language.
What do we have so far:

                                  | Interepreter | Stack Machine | x86 |
--------------------------------- | ------------ | ------------- | --- |
binops                            | [x]          | [x]           | [x] |
if/while/for/repeat control flows | [x]          | [x]           | [x] |
funcs                             | [x]          | [x]           | [x] |

# TODO

- [ ] переписать !! and && в x86 для ускорения
- [ ] избавиться от второго аругмента в call на стадии fdefs (?)
- [ ] чистить стэк после вызова функции как процедуры в ST
- [ ] HashMap вместо Map (?)
- [ ] сделать рефакторинг
- [x] перепройти все тесты
- [ ] переделать функции в SM и Int правильно? и с классами env
- [x] сохранять регистры после вызова функции
- [x] добавить регистры в рабочий стек по x86
- [ ] перейти на docker + vagrant (?)
