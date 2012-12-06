<?php

/**
 * Implements hook_features_api().
 */
function featuresets_features_api() {
	return array(
			'field_base' => array(
					'name' => t('Field Base'),
					'default_hook' => 'field_default_field_bases',
					'default_file' => FEATURES_DEFAULTS_INCLUDED,
					'feature_source' => TRUE,
					'file' => drupal_get_path('module', 'featuresets') . '/featuresets.base.inc',
			),
			'field_instance' => array(
					'name' => t('Fields Instance'),
					'default_hook' => 'field_default_field_instances',
					'default_file' => FEATURES_DEFAULTS_INCLUDED,
					'feature_source' => TRUE,
					'file' => drupal_get_path('module', 'featuresets') . '/featuresets.instance.inc',
			),
	);
}

/**
 * Implements hook_features_api_alter().
 */
function featuresets_features_api_alter (&$components) {
	unset($components['field']);
}

/**
 * Implements hook_features_pipe_COMPONENT_alter().
 */
function featuresets_features_pipe_node_alter(&$pipe, $data, &$export) {
	foreach($pipe['field'] as $instance) {
		$fragments = explode('-', $instance);
		array_shift($fragments);
		array_shift($fragments);
		$base_field = implode('-', $fragments);
		if (!isset($export['features']['field_instance'])) {
			$export['features']['field_instance'] = array();
		}
		if (!isset($export['features']['field_base'])) {
			$export['features']['field_base'] = array();
		}
		$pipe['field_instance'][] = $instance;
		$export['features']['field_instance'][$instance] = $instance;
		$pipe['field_base'][] = $base_field;
		$export['features']['field_base'][$base_field] = $base_field;

	}
	unset($pipe['field']);
}

/**
 * Implements hook_features_export_render().
 * 
 * Sets warning when the deprecated COMPONENT.features.field.inc is detected
 */
function featuresets_features_export_render($module_name, $data, $export = NULL) {
	$code = array();
	$code[] = '$mycomponents = array();';
	foreach ($data as $name) {
		$code[] = "  \$mycomponents['{$name}'] = " . features_var_export(mycomponent_load($name)) .";";
	}
	$deprecated = array(
			"{$module_name}.features.field"
	);
	foreach (file_scan_directory(drupal_get_path('module', $module_name), '/.*/') as $file) {
		if (in_array($file->name, $deprecated, TRUE)) {
			features_log(t('The file @filename has been deprecated and can be removed.', array('@filename' => $file->filename)), 'status');
		}
	}

	$code[] = "return \$mycomponents;";
	$code = implode("\n", $code);
	return array('mycomponent_defaults' => $code);
}
