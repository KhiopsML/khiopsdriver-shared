# Integration tests

Integration test suite for the drivers which common interface is defined [here](./plugin.hpp).

> **IMPORTANT**: The environment variable `STORAGE_DRIVER_NAME` must be defined prior to running CMake.
> The supported values are: `S3`, `GCS` and `AZURE`.