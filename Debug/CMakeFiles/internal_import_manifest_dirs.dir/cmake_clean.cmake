FILE(REMOVE_RECURSE
  "_CPack_Packages"
  "CMakeFiles/internal_import_manifest_dirs"
  "manifests/default/MANIFEST.json"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/internal_import_manifest_dirs.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)
