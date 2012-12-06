<?php

function field_instance_features_export_options() {
	return field_features_export_options();
}

/**
 * Implements hook_features_export().
 */
function field_instance_features_export($data, &$export, $module_name = '') {
	$pipe = array('field_base' => array());
	$map = features_get_default_map('field');

	// The field_default_fields() hook integration is provided by the
	// features module so we need to add it as a dependency.
	$export['dependencies']['features'] = 'features';

	foreach ($data as $identifier) {
		if ($field = featuresets_field_instance_load($identifier)) {
			$export['features']['field_instance'][$identifier] = $identifier;
			$export['dependencies'][$field['field_instance']['widget']['module']] = $field['field_instance']['widget']['module'];
			foreach ($field['field_instance']['display'] as $key => $display) {
				if (isset($display['module'])) {
					$export['dependencies'][$display['module']] = $display['module'];
					// @TODO: handle the pipe to image styles
				}
			}
			$pipe['field_base'][$field['field_config']['field_name']] = $field['field_config']['field_name'];
		}
	}
	return $pipe;
}

/**
 * Implements hook_features_export_render().
 */
function field_instance_features_export_render($module, $data, $export = NULL) {
	$translatables = $code = array();
	$code[] = '  $field_instances = array();';
	$code[] = '';
	foreach ($data as $identifier) {
		if ($field = featuresets_field_instance_load($identifier)) {
			$instance = $field['field_instance'];
			_field_features_export_sort($instance);
			$field_export = features_var_export($instance, '  ');
			$field_identifier = features_var_export($identifier);
			$code[] = "  // Exported field_instance: {$field_identifier}";
			$code[] = "  \$field_instances[{$field_identifier}] = {$field_export};";
	  $code[] = "";

	  if (!empty($field['field_instance']['label'])) {
	  	$translatables[] = $field['field_instance']['label'];
	  }
	  if (!empty($field['field_instance']['description'])) {
	  	$translatables[] = $field['field_instance']['description'];
	  }
		}
	}
	if (!empty($translatables)) {
		$code[] = features_translatables_export($translatables, '  ');
	}
	$code[] = '  return $field_instances;';
	$code = implode("\n", $code);
	return array('field_default_field_instances' => $code);
}

// Helper to enforce consistency in field export arrays.
/**
 * Implements hook_features_revert().
 */
function field_instance_features_revert($module) {
	field_instance_features_rebuild($module);
}

/**
 * Implements of hook_features_rebuild().
 * Rebuilds fields from code defaults.
 */
function field_instance_features_rebuild($module) {
	if ($fields = features_get_default('field_instance', $module)) {
		field_info_cache_clear();
		$existing_instances = field_info_instances();
		foreach ($fields as $field_instance) {
			// If the field base information does not exist yet, cancel out.
			if (!field_info_field($field_instance['field_name'])) {
				continue;
			}
			
			// Create or update field instance.
			if (isset($existing_instances[$field_instance['entity_type']][$field_instance['bundle']][$field_instance['field_name']])) {
				$existing_instance = $existing_instances[$field_instance['entity_type']][$field_instance['bundle']][$field_instance['field_name']];
				if ($field_instance + $existing_instance != $existing_instance) {
					field_update_instance($field_instance);
				}
			}
			else {
				field_create_instance($field_instance);
				$existing_instances[$field_instance['entity_type']][$field_instance['bundle']][$field_instance['field_name']] = $field_instance;
			}
		}
			
		if ($fields) {
			variable_set('menu_rebuild_needed', TRUE);
		}
	}
}


/**																											* Load a field's configuration and instance configuration by an
 * entity_type-bundle-field_name identifier.
 */
function featuresets_field_instance_load($identifier) {
	list($entity_type, $bundle, $field_name) = explode('-', $identifier);
	$field_info = field_info_field($field_name);
	$instance_info = field_info_instance($entity_type, $field_name, $bundle);
	if ($field_info && $instance_info) {
		unset($field_info['id']);
		unset($field_info['bundles']);
		unset($instance_info['id']);
		unset($instance_info['field_id']);
		return array(
				'field_config' => $field_info,
				'field_instance' => $instance_info,
		);
	}
	return FALSE;
}
