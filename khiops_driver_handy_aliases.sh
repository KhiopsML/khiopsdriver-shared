# Generate build system for debug configuration with tests enabled
alias khiopsdrivergendbg='cmake --preset ninja-dbg -DBUILD_TESTS=ON'

# Build using debug configuration
alias khiopsdriverbuilddbg='cmake --build --preset ninja-dbg -j'

# Run tests using debug configuration, stopping on the first failure
alias khiopsdrivertestdbg='ctest --preset ninja-dbg --stop-on-failure'

# S3 driver aliases
alias s3gendbg='khiopsdrivergendbg'
alias s3builddbg='khiopsdriverbuilddbg'
alias s3testdbg='STORAGE_DRIVER_TEST_URL_PREFIX=s3://diod-data-di-jupyterhub khiopsdrivertestdbg'

# GCS driver aliases
alias gcsgendbg='khiopsdrivergendbg'
alias gcsbuilddbg='khiopsdriverbuilddbg'
alias gcstestdbg='STORAGE_DRIVER_TEST_URL_PREFIX=gs://data-test-khiops-driver-gcs khiopsdrivertestdbg'

# Azure driver aliases
alias azgendbg='khiopsdrivergendbg'
alias azbuilddbg='khiopsdriverbuilddbg'
alias azetestdbg="AZURE_EMULATED_STORAGE=true \
    AZURE_STORAGE_CONNECTION_STRING='DefaultEndpointsProtocol=http;AccountName=devstoreaccount1;AccountKey=Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw==;BlobEndpoint=http://localhost:10000/devstoreaccount1;' \
    STORAGE_DRIVER_TEST_URL_PREFIX=http://localhost:10000/devstoreaccount1/data-test-khiops-driver-azure khiopsdrivertestdbg"
alias azbtestdbg='STORAGE_DRIVER_TEST_URL_PREFIX=https://khiopsdriverazure.blob.core.windows.net/data-test-khiops-driver-azure khiopsdrivertestdbg'
alias azftestdbg='STORAGE_DRIVER_TEST_URL_PREFIX=https://khiopsdriverazure.file.core.windows.net/data-test-khiops-driver-azure khiopsdrivertestdbg'