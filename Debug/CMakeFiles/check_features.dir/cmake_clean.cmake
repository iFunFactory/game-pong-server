FILE(REMOVE_RECURSE
  "_CPack_Packages"
  "CMakeFiles/check_features"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/check_features.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
