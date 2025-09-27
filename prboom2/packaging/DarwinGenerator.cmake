set(staging_dir "${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGING_INSTALL_PREFIX}")
set(packaged_dir "${CPACK_TEMPORARY_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}")
file(MAKE_DIRECTORY ${packaged_dir})

file(COPY_FILE
  ${staging_dir}/${CPACK_BIN_DIR}/nyan-doom
  ${packaged_dir}/nyan-doom
)

file(COPY_FILE
  ${staging_dir}/${CPACK_PWAD_DIR}/nyan-doom.wad
  ${packaged_dir}/nyan-doom.wad
)

file(COPY_FILE
  ${staging_dir}/${CPACK_LICENSE_DIR}/COPYING
  ${packaged_dir}/COPYING.txt
)

file(WRITE
  "${packaged_dir}/Troubleshooting.txt"
  "If you are getting errors like 'libzip.5.5.dylib cant be opened because Apple cannot check it for malicious software.'\n"
  "Run the following command in the nyan-doom folder:\n\n"
  "xattr -dr com.apple.quarantine path/to/folder\n"
)

find_program(DYLIBBUNDLER_EXECUTABLE
  NAMES dylibbundler
  REQUIRED
)

execute_process(
  COMMAND ${DYLIBBUNDLER_EXECUTABLE}
    --bundle-deps
    --create-dir
    --overwrite-files
    --fix-file ${packaged_dir}/nyan-doom
    --install-path @executable_path/libs_${CPACK_SYSTEM_PROCESSOR}
    --dest-dir ${packaged_dir}/libs_${CPACK_SYSTEM_PROCESSOR}
)

execute_process(
  COMMAND zip
    -r ${CPACK_PACKAGE_DIRECTORY}/${CPACK_PACKAGE_FILE_NAME}.zip
    ${CPACK_PACKAGE_FILE_NAME}
  WORKING_DIRECTORY ${CPACK_TEMPORARY_DIRECTORY}
)
