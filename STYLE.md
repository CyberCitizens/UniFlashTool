# Conventions and style

Hey !

If you're reading this, you're probably looking to contribute to the project, or you're just curious about whatever the heck I wrote in this repository.

Good news, this file exist just for that !!!

## Conventions

I'm using an in-between the Unix and Microsoft object-oriented convention:

- top-namespace is lowercase,
- sub-namespaces are UpperCamelCase.
- Global constants are PASCAL_CASE,
- Disposable variables are \_camelCase, prefixed with an underscore (\_).s
- Methods and functions follow the same style, they're named in UpperCamelCase.
	- *If you didn't know, methods are the functions inside a class, while functions aren't related
	to objects !*
- I use sneaky_case only for C-only implementations among all C++ code

## Style

I prefer to align my code using tabs, to keep same-characters on the same start and ending columns.

But as long as your code is correctly indented, do as you wish !

Also, I usually put my brace after a newline character in my body declarations:
```cpp
void LikeThis()
{
	// ...
}

void NotLikeThis() {
	// ...
}
```

But it's okay if you don't, I myself get sometimes confused and put it the "wrong" (for me) way.

Anyway, thanks for having that read done ! It's really useful if you wish to contribute, or if you want to understand the code more easily. It's really important to me that my program is easily accessible for everyone.
