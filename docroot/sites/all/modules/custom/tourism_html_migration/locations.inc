<?php

class TourismHtmlLocationNodeMigration extends TourismHtmlMigration {
  public $base_dir;
  

  /**
   * Constructor.
   */
  public function __construct($arguments = array()) {
    $this->arguments = $arguments;
    parent::__construct($arguments);
    $this->description =
      t('Migrate nodes from HTML elements');
    //has_krumo();
    //drush_print_r($this);
    

    // A map of source HTML filename -> destination node id.
    /*$this->map = new MigrateSQLMap($this->currentId,
        array(
          'sourceid' => array(
            'type' => 'varchar',
            'length' => 255,
            'not null' => TRUE,
          )
        ),
        MigrateDestinationNode::getKeySchema()
    );*/

    // The source fields.
    $fields = array(
      'title' => t('Title'),
      'name' => t('Name'),
      'street' => t('Street'),
      'city' => t('City'),
      'province' => t('Province'),
      'additional' => t('Additional'),
      'postal_code' => t('Postal Code')
    );

    // get the html files
    $options['file'] = drupal_get_path('module', 'tourism_html_migration') . '/legacy/locations.html';
    
    $this->source = new HtmlMigrateLocationSource($fields, $options);

    // The destination is the mynode content type.
    $this->destination = new MigrateDestinationNode('ol_locator_location');
    
    $this->map = new MigrateSQLMap($this->machineName, 
                array(    
                    'id' => array(    
                        'type' => 'int', 
                        'not null' => TRUE, 
                        'description' => 'Location ID.', 
                        'alias' => 'id'   
                    ) 
                ), 
                //schema definition for the primary key of the destination 
                MigrateDestinationNode::getKeySchema()    
        );                                              

    // Map the fields, pretty straightforward in this case.
    //$this->addFieldMapping('uid', 'uid');
    $this->addFieldMapping('status')
      ->defaultValue(1);
    $this->addFieldMapping('title', 'title');

    $this->addFieldMapping('field_ol_locator_address', 'country');
    $this->addFieldMapping('field_ol_locator_address:country', 'country')
      ->defaultValue('US');
    $this->addFieldMapping('field_ol_locator_address:name_line', 'name');
    $this->addFieldMapping('field_ol_locator_address:thoroughfare', 'street');
    $this->addFieldMapping('field_ol_locator_address:locality', 'city');
    $this->addFieldMapping('field_ol_locator_address:administrative_area', 'state');
    $this->addFieldMapping('field_ol_locator_address:premise', 'additional');
    $this->addFieldMapping('field_ol_locator_address:postal_code', 'postal_code');
    
  }

  /**
   * Prepare a row.
   */
  public function prepareRow($row) {
    // Set to admin for now.
    $row->uid = 1;
    $row->status = 1;
    $row->country = array("US");

    //drush_print_r($row);

  }
}

class HtmlMigrateLocationSource extends MigrateSource {
  protected $currentId;
  protected $numRows;
  protected $source_parser;

  // Your constructor will initialize any parameters to your migration. It's
  // important to pass through the options, so general options such as
  // cache_counts will work.
  public function __construct($num_rows = array(), array $options = array()) {
    parent::__construct($options);
    
    $this->source_parser = new SourceParser('HtmlMigrateMeetingSource', 'locations', file_get_contents($options['file']));
    $this->numRows = $this->source_parser->getNumRows();
    //drush_print_r($this->numRows);
  }

  /**
   * Return a string representing the source, for display in the UI.
   */
  public function __toString() {
    return t('Generate %num sample rows', array('%num' => $this->numRows));
  }

  /**
   * Returns a list of fields available to be mapped from the source,
   * keyed by field name.
   */
  public function fields() {
    return array(
          'id' => t('ID'),
          'title' => t('Title'),
          'agenda' => t('Agenda'),
          'minutes' => t('Minutes'),
          'date' => t('Date'),
    );
  }
  
  /**
   * Return the number of available source records.
   */
  public function computeCount() {
    return $this->numRows;
  }

  /**
   * Do whatever needs to be done to start a fresh traversal of the source data.
   *
   * This is always called at the start of an import, so tasks such as opening 
   * file handles, running queries, and so on should be performed here.
   */
  public function performRewind() {
    $this->currentId = 1;
  }

  /**
   * Fetch the next row of data, returning it as an object. Return FALSE
   * when there is no more data available.
   */
  public function getNextRow() {
    if ($this->currentId <= $this->numRows) {

      $row = new stdClass;
      $row->id = $this->currentId;
      
      //$source_parser = new SourceParser(0, file_get_contents($options['file']));
      $node = $this->source_parser->getNode($row->id);
      
    
      $row->title = $node['name'];
      $row->name = $node['name'];
      $row->street = $node['street'];
      $row->additional = $node['additional'];
      $row->city = $node['city'];
      $row->state = $node['state'];
      $row->postal_code = $node['postal_code'];

      
      //drush_print_r($row);
      
      $this->currentId++;
      return $row;
    }
    else {
      return NULL;
    }
  }
}