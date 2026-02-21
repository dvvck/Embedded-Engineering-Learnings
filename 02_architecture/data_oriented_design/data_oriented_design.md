# Data oriented Design

> The purpose of all programs, and all parts of those programs, is to transform data from one form to another. 

> Solving problems you don`t have creates more problems you definitely do.

Data oriented Design puts a clear focus on the performance of software.
It tries to optimize the code in order to work well with the platform and kind off represents a counter design for object oriented Programming and some of the Clean Code principles of Martin Fowler.

## Key Takeaways

- When performance matters consider data oriented design
- don't try to solve every problem with the same tool
- benchmark performance to evaluate performance critical code
- use OOP on a higher hierarchy and DOD in a lower performance critical hierarchy

## Summary

### CppCon 2014: Mike Acton "Data-Oriented Design and C++"[^1]

Mike Acton is a game developer and part of the team that focuses on the performance critical aspects of games like shaders, rendering, animations etc..
In his talk he highlights the importance of optimizing software for the hardware it is supposed to work with.
Especially the CPU cache and stream of data through the memory.

He argues that the compiler needs as much information as possible in order to be able to optimize the instructions the CPU has to make optimally.
The optimization focuses on the way the data is stored in memory since the time it takes to process data from the RAM compared to the cache is 100-200x higher.
He shows multiple examples of different C++ functions and the equivalent assembly after compiling it.

A key takeaway from those examples is that booleans or how Mike Action likes to call them "last minute decisions" are an indicator for poorly optimized functions and should be pulled out of loops.
It is important to say that most of the optimizations where made in some kind of loops.

Using multiple specialized functions instead of deciding inside a generic function improve performance.
Furthermore it can be helpful when working with complex object to extract the important data into a local structure and only loop over that in order to increase the density of relevant information in each cache line.

Talking about information density, booleans aren`t very dense in information.
A boolean has the information if one bit but is stored in one byte therefore wasting seven bit.
Seven bit doesn't sound like a lot but it can add up and if those seven bit result in pushing important information out of the cache resulting in a cache miss that a significant performance loss.

###  More Speed & Simplicity: Practical Data-Oriented Design in C++ - Vittorio Romeo - CppCon 2025[^2]

Vittorio Romeo was heavily inspired by Mike Actons talk and therefore covers almost the exact same topic.
Nevertheless I really enjoyed Vittorios keynote because I felt like his examples where easier to follow and easier to remember.
Furthermore I liked how differentiated he was about the applicability of data oriented design and acknowledged that OOP isn't the root cause of bad software design but rather trying to solve every problem with OOP is a bad habit.  
Just like trying to drill a hole with a hammer is a bad.



## Sources

[^1]:[CppCon 2014: Mike Acton "Data-Oriented Design and C++"](https://www.youtube.com/watch?v=rX0ItVEVjHc)

[^2]:[More Speed & Simplicity: Practical Data-Oriented Design in C++ - Vittorio Romeo - CppCon 2025](https://www.youtube.com/watch?v=SzjJfKHygaQ&t=1s)
