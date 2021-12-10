# Thread-Man

Instructions for building:

`make` to build, `make clean` to clean, and `make run` does both and then runs the executable. Requires SDL2.

Hopefully this will compile on a newer gcc version. I made this on my desktop rather than my laptop, and new gcc didn't want to work. Hence why I had to use std::experimental::filesystem.

This was a really cool project to work on. I had no idea the AI for the ghosts was so intricate. Initially I was going to go way more object oriented for this project, but I drew on some of my experience from game jams and kept it simple with just a few global variables and a basic entity data type.
Because I built everything that changed out of the same entity type, it was easy to keep things thread-safe, just by including a mutex in the entity structure. This kept me from having to reinvent the wheel with multiple different object oriented classes. Really all the "entities" were just sprites anyway, so there was hardly need for anything much more complicated.
The only things that needed AI at all were the ghosts, and I was able to repurpose a lot of the pac-man control code once I figured out how their pathfinding was supposed to work. As for synchronizing the threads with each other, a few atomic variables was all it took to keep the ghosts all on the same page about what "mode" they were all in, and when the threads needed to exit.
I also found a cool new library in the form of SDL. I'd always heard of it before but never tried it out, and I'm interested to see what the Gobject bindings are like for it. It seems like it has a lot of cool features that could make it even more attractive than GLFW, which is what I've used for this sort of thing in the past.
