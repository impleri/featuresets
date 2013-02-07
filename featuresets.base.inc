<?php


/**
 * Implements hook_features_export_options().
 */
function field_base_features_export_options() {
  $options = array();
  foreach (field_info_fields() as $field_name => $info) {
    $options[$field_name] = $field_name;
  }
  return $options;
}

/**
 * Implements hook_features_export().
 */
function field_base_features_export($data, &$export, $module_name = '') {
  $pipe = array();
  $map = features_get_default_map('field_base');

  // The field_default_field_bases() hook integration is provided by the
  // features module so we need to add it as a dependency.
  $export['dependencies']['features'] = 'features';

  foreach ($data as $identifier) {
    if ($field = featuresets_field_base_load($identifier)) {
      // If this field is already provided by another module, remove the field
      // and add the other module as a dependency.
      if (isset($map[$identifier]) && $map[$identifier] != $module_name) {
        if (isset($export['features']['field_base'][$identifier])) {
          unset($export['features']['field_base'][$identifier]);
        }
        $module = $map[$identifier];
        $export['dependencies'][$module] = $module;
      }
      // If the field has not yet been exported, add it
      else {
        $export['features']['field_base'][$identifier] = $identifier;
        $export['dependencies'][$field['module']] = $field['module'];
        if ($field['storage']['type'] != variable_get('field_storage_default', 'field_sql_storage')) {
          $export['dependencies'][$field['storage']['module']] = $field['storage']['module'];
        }
        // If taxonomy field, add in the vocabulary
        if ($field['type'] == 'taxonomy_term_reference' && !empty($field['settings']['allowed_values'])) {
          foreach ($field['settings']['allowed_values'] as $allowed_values) {
            if (!empty($allowed_values['vocabulary'])) {
              $pipe['taxonomy'][] = $allowed_values['vocabulary'];
            }
          }
        }
      }
    }
  }
  return $pipe;
}

/**
 * Implements hook_features_export_render().
 */
function field_base_features_export_render($module, $data, $export = NULL) {
  $code = array();
  $code[] = '  $field_bases = array();';
  $code[] = '';
  foreach ($data as $identifier) {
    if ($field = featuresets_field_base_load($identifier)) {
      unset($field['columns']);
      // unset($field['locked']);
      // Only remove the 'storage' declaration if the field is using the default
      // storage type.
      if ($field['storage']['type'] == variable_get('field_storage_default', 'field_sql_storage')) {
        unset($field['storage']);
      }
      // If we still have a storage declaration here it means that a non-default
      // storage type was altered into to the field definition. And noone would
      // never need to change the 'details' key, so don't render it.
      if (isset($field['storage']['details'])) {
        unset($field['storage']['details']);
      }

      _featuresets_export_sort($field);
      $field_export = features_var_export($field, '  ');
      $field_identifier = features_var_export($identifier);
      $code[] = "  // Exported field_base: {$field_identifier}";
      $code[] = "  \$field_bases[{$field_identifier}] = {$field_export};";
      $code[] = "";
    }
  }
  $code[] = '  return $field_bases;';
  $code = implode("\n", $code);
  return array('field_default_field_bases' => $code);
}

// Helper to enforce consistency in field export arrays.
/**
 * Implements hook_features_revert().
 */
function field_base_features_revert($module) {
  field_base_features_rebuild($module);
}

/**
 * Implements of hook_features_rebuild().
 * Rebuilds fields from code defaults.
 */
function field_base_features_rebuild($module) {
  if ($fields = features_get_default('field_base', $module)) {
    field_info_cache_clear();

    // Load all the existing field bases up-front so that we don't
    // have to rebuild the cache all the time.
    $existing_fields = field_info_fields();

    foreach ($fields as $field) {
      // Create or update field.
      if (isset($existing_fields[$field['field_name']])) {
        $existing_field = $existing_fields[$field['field_name']];
        if ($field + $existing_field != $existing_field) {
          field_update_field($field);
        }
      }
      else {
        field_create_field($field);
        $existing_fields[$field['field_name']] = $field;
      }
      variable_set('menu_rebuild_needed', TRUE);
    }
  }
}

/**
 * Load a field's base information, removing uneeded information.
 */
function featuresets_field_base_load($field_name) {
  if ($field_info = field_info_field($field_name)) {
    unset($field_info['id']);
    unset($field_info['bundles']);
    return $field_info;
  }
  return FALSE;
}
