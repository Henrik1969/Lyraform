# Pattern Explored

This area contains earlier pattern experiments and prototypes that informed
Flowmini and Lyraform design.

They are not the current active Lyraform implementation, but remain useful as
implementation archaeology and design reference.

## Reproducible experiment build

The original single-file experiments can be rebuilt without writing binaries
into the source directory:

```sh
./Pattern_explored/src/build.sh
```

Outputs and compiler logs are placed in `Pattern_explored/build/`. Use
`--build-dir DIR` to select another output directory. The generated binaries
are intentionally not tracked.
