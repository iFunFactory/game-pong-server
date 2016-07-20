FILE(REMOVE_RECURSE
  "_CPack_Packages"
  "CMakeFiles/internal_import_resource_dirs"
  "resources/game_data"
  "resources/client_data"
  "resources/json_protocols"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/internal_import_resource_dirs.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
