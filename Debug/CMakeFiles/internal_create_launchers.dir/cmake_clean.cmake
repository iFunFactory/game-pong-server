FILE(REMOVE_RECURSE
  "_CPack_Packages"
  "CMakeFiles/internal_create_launchers"
  "pong-launcher"
  "pong-local"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/internal_create_launchers.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
