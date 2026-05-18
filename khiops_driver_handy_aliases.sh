# Generate build system for debug configuration with tests enabled
alias khiopsdrivergendbg='cmake --preset ninja-dbg -DBUILD_TESTS=ON'
# Generate build system for release configuration with tests enabled
alias khiopsdrivergenrel='cmake --preset ninja-rel -DBUILD_TESTS=ON'

# Build using debug configuration
alias khiopsdriverbuilddbg='cmake --build --preset ninja-dbg -j'
# Build using release configuration
alias khiopsdriverbuildrel='cmake --build --preset ninja-rel -j'

# Run tests using debug configuration, stopping on the first failure
alias khiopsdrivertestdbg='ctest --preset ninja-dbg --stop-on-failure'
# Run tests using release configuration, stopping on the first failure
alias khiopsdrivertestrel='ctest --preset ninja-rel --stop-on-failure'

# S3 driver aliases
alias s3gendbg='khiopsdrivergendbg'
alias s3genrel='khiopsdrivergenrel'
alias s3builddbg='khiopsdriverbuilddbg'
alias s3buildrel='khiopsdriverbuildrel'
alias s3testdbg='STORAGE_DRIVER_TEST_URL_PREFIX=s3://diod-data-di-jupyterhub khiopsdrivertestdbg'
alias s3testrel='STORAGE_DRIVER_TEST_URL_PREFIX=s3://diod-data-di-jupyterhub khiopsdrivertestrel'

# GCS driver aliases
alias gcsgendbg='khiopsdrivergendbg'
alias gcsgenrel='khiopsdrivergenrel'
alias gcsbuilddbg='khiopsdriverbuilddbg'
alias gcsbuildrel='khiopsdriverbuildrel'
alias gcstestdbg='STORAGE_DRIVER_TEST_URL_PREFIX=gs://data-test-khiops-driver-gcs khiopsdrivertestdbg'
alias gcstestrel='STORAGE_DRIVER_TEST_URL_PREFIX=gs://data-test-khiops-driver-gcs khiopsdrivertestrel'

# Azure driver aliases
alias azgendbg='khiopsdrivergendbg'
alias azgenrel='khiopsdrivergenrel'
alias azbuilddbg='khiopsdriverbuilddbg'
alias azbuildrel='khiopsdriverbuildrel'
alias azetestdbg="AZURE_EMULATED_STORAGE=true \
    AZURE_STORAGE_CONNECTION_STRING='DefaultEndpointsProtocol=http;AccountName=devstoreaccount1;AccountKey=Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw==;BlobEndpoint=http://localhost:10000/devstoreaccount1;' \
    STORAGE_DRIVER_TEST_URL_PREFIX=http://localhost:10000/devstoreaccount1/data-test-khiops-driver-azure khiopsdrivertestdbg"
alias azetestrel="AZURE_EMULATED_STORAGE=true \
    AZURE_STORAGE_CONNECTION_STRING='DefaultEndpointsProtocol=http;AccountName=devstoreaccount1;AccountKey=Eby8vdM02xNOcqFlqUwJPLlmEtlCDXJ1OUzFT50uSRZ6IFsuFq2UVErCz4I6tq/K1SZFPTOtr/KBHBeksoGMGw==;BlobEndpoint=http://localhost:10000/devstoreaccount1;' \
    STORAGE_DRIVER_TEST_URL_PREFIX=http://localhost:10000/devstoreaccount1/data-test-khiops-driver-azure khiopsdrivertestrel"
alias azbtestdbg='STORAGE_DRIVER_TEST_URL_PREFIX=https://khiopsdriverazure.blob.core.windows.net/data-test-khiops-driver-azure khiopsdrivertestdbg'
alias azbtestrel='STORAGE_DRIVER_TEST_URL_PREFIX=https://khiopsdriverazure.blob.core.windows.net/data-test-khiops-driver-azure khiopsdrivertestrel'
alias azftestdbg='STORAGE_DRIVER_TEST_URL_PREFIX=https://khiopsdriverazure.file.core.windows.net/data-test-khiops-driver-azure khiopsdrivertestdbg'
alias azftestrel='STORAGE_DRIVER_TEST_URL_PREFIX=https://khiopsdriverazure.file.core.windows.net/data-test-khiops-driver-azure khiopsdrivertestrel'