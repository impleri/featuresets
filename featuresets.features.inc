<?php

/**
 * Implements hook_features_api().
 */
function featuresets_features_api() {
	return array(
			'field_base' => array(
					'name' => t('Field Bases'),
					'default_hook' => 'field_default_field_bases',
					'default_file' => FEATURES_DEFAULTS_INCLUDED,
					'feature_source' => TRUE,
					'file' => drupal_get_path('module', 'featuresets') . '/featuresets.base.inc',
			),
			'field_instance' => array(
					'name' => t('Field Instances'),
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
	$components['field']['field']['feature_source'] = false;
}

/**
 * Implements hook_features_pipe_COMPONENT_alter().
 */
function featuresets_features_pipe_node_alter(&$pipe, $data, $export) {
	foreach($pipe['field'] as $instance) {
		$fragments = explode('-', $instance);
		$base_field = array_pop($fragments);
		if (!isset($pipe['field_instance'])) {
		  $pipe['field_instance'] = array();
		}
		if (!isset($pipe['field_base'])) {
		  $pipe['field_base'] = array();
		}
		$pipe['field_instance'][] = $instance;
		$pipe['field_base'][] = $base_field;
	}
	unset($pipe['field']);
}

// function featuresets_features_pipe_alter(&$pipe, $data, $export) {
//   if ($export['component'] == 'node') {
//     featuresets_features_pipe_node_alter($pipe, $data, $export);
//   }
// }

function featuresets_features_export_alter(&$export, $module_name) {
	$deprecated = array(
			"{$module_name}.features.field"
	);
	foreach (file_scan_directory(drupal_get_path('module', $module_name), '/.*/') as $file) {
		if (in_array($file->name, $deprecated, TRUE)) {
			features_log(t('The file @filename has been deprecated and can be removed.', array('@filename' => $file->filename)), 'status');
		}
	}
}

// Helper to enforce consistency in field export arrays.
function _featuresets_export_sort(&$field, $sort = TRUE) {

  // Some arrays are not sorted to preserve order (for example allowed_values).
  static $sort_blacklist = array(
      'allowed_values',
  );

  if ($sort) {
    ksort($field);
  }
  foreach ($field as $k => $v) {
    if (is_array($v)) {
      _featuresets_export_sort($field[$k], !in_array($k, $sort_blacklist));
    }
  }
}
