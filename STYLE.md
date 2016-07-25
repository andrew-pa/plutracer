# style in plutracer

+ This is C++14, use C++14! no silly C++98 junk
+ Keep braces in K&R style

	````
	int foo() {
		// .. code here ..
	}
	````
+ Use underscore_style for names

+ Add documentation when necessary, but don't be redundant

	If you can make the code say it all, do, and avoid any documentation at all

+ Don't worry about silly OOP encapsulation stuff

	C++ is dangerous, that's why we like it. It's ok to have public data members, even ones that aren't const
	That also means that unless desperatly necessary, there should be no property functions (accessors/setters)
	
+ If you don't know what the memory ownership of something will be and you don't need a lot of pointer math (or maybe even if you do), use shared_ptr, unique_ptr, vector to manage memory. STL is good, use the STL


