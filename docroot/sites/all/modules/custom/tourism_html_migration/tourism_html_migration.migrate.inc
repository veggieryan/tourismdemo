<?php

// Include QueryPath.
require_once 'querypath/src/qp.php';

/*
 * You must implement hook_migrate_api(), setting the API level to 2, for
 * your migration classes to be recognized by the Migrate module.
 */
function tourism_html_migration_migrate_api() {
  $api = array(
    'api' => 2,
    'groups' => array(
      'tourism_html' => array(
        'title' => t('Tourism HTML'),
      ),
    ),
    //specify migrations
    'migrations' => array(
      'Locations' => array(
        'class_name' => 'TourismHtmlLocationNodeMigration',
        'group_name' => 'tourism_html',
      ),
    ),  
  );
  return $api;
}

abstract class TourismHtmlMigration extends Migration {
  public function __construct($arguments = array()) {
    $this->arguments = $arguments;
    // Always call the parent constructor first for basic setup
    parent::__construct($arguments);
  }
}

