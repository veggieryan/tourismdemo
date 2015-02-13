<?php


class SourceParser {
  protected $id;
  protected $type;
  protected $html;
  protected $qp;
  protected $nodes;

  /**
   * Constructor.
   *
   * @param $id
   *  The filename, e.g. pm7205.html
   * @param $html
   *  The full HTML data as loaded from the file.
   */
  public function __construct($id, $type, $html) {
    $this->id = $id;
    $this->type = $type;
    $this->html = $html;
    
    //drush_print_r('id: '. $id);
    //drush_print_r('html: '. $this->html);

    $this->charTransform();
    $this->fixEncoding();
    $this->wrapHTML();
    $this->initQP();
    $this->stripComments();
  }

  /**
   * Replace characters.
   */
  protected function charTransform() {
    // We need to strip the Windows CR characters, because otherwise we end up
    // with &#13; in the output.
    // http://technosophos.com/content/querypath-whats-13-end-every-line
    $this->html = str_replace(chr(13), '', $this->html);
  }

  /**
   * Deal with encodings.
   */
  protected function fixEncoding() {
    // If the content is not UTF8, we assume it's WINDOWS-1252. This fixes
    // bogus character issues. Technically it could be ISO-8859-1 but it's safe
    // to convert this way.
    // http://en.wikipedia.org/wiki/Windows-1252
    $enc = mb_detect_encoding($this->html, 'UTF-8', TRUE);
    if (!$enc) {
      $this->html = mb_convert_encoding($this->html, 'UTF-8', 'WINDOWS-1252');
    }
  }

  /**
   * Wrap an HTML fragment in the correct head/meta tags so that UTF-8 is
   * correctly detected, and for the parsers and tidiers.
   */
  protected function wrapHTML() {
    // We add surrounding <html> and <head> tags.
    $html = '<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">';
    $html .= '<html xmlns="http://www.w3.org/1999/xhtml"><head><meta http-equiv="Content-Type" content="text/html; charset=utf-8" /></head><body>';
    $html .= $this->html;
    $html .= '</body></html>';
    $this->html = $html;
  }

  /**
   * Create the QueryPath object.
   */
  protected function initQP() {
    $qp_options = array(
      'convert_to_encoding' => 'utf-8',
      'convert_from_encoding' => 'utf-8',
      'strip_low_ascii' => FALSE,
    );
    $this->qp = htmlqp($this->html, NULL, $qp_options);
    //drush_print_r($this->qp->find('#board-administration-meeting tbody')->children());
  }

  /**
   * Remove the comments from the HTML.
   */
  protected function stripComments() {
    foreach ($this->qp->top()->xpath('//comment()')->get() as $comment) {
      $comment->parentNode->removeChild($comment);
    }
  }
  
  /**
   * Return the nodes.
   */
  public function getNode($id) {
    
    if (empty($this->nodes)) {
      $nodes = $this->getNodes();
    }  
    
    return $this->nodes[$id];
  }
  
  /**
   * Return the nodes.
   */
  public function getNodes() {
    
    if (empty($this->nodes)) {
      $body = $this->qp->top('body')->innerHTML();
      
      $qp = $this->qp->find('#locations-wrapper tbody')->children();
      //drush_print_r($qp);

      $i = 0;

      foreach ($qp as $child) {
        $tds = $child->children('td');
        //drush_print_r($child);
        //two different table styles for meetings
        if (count($tds) == 2) {
          foreach ($tds as $key => $td) {
            switch ($key) {
              
              case 1: 
                $address = explode('<br />', $td->innerHtml());
                if (substr($address[0], 0, strlen('<font size="-1">')) == '<font size="-1">') {
                    $address[0] = substr($address[0], strlen('<font size="-1">'));
                } 
                if (count($address) == 3) {
                  $nodes[$i]['name'] = $address[0];
                  
                  $nodes[$i]['street'] = $address[1];
                  $address[2] = preg_replace('/\<\/font\>/', '', $address[2]);
                  $city_array = explode(',', $address[2]);
                  $city = $city_array[0];
                  $nodes[$i]['city'] = $city;
                  preg_match('/([A-Z]{2})/', $city_array[1], $matches);
                  $nodes[$i]['state'] = $matches[0];
                  preg_match('/(\d{5})/', $city_array[1], $matches);
                  $nodes[$i]['postal_code'] = $matches[0];
                  $nodes[$i]['additional'] = NULL;
                 
                  
                }
                if (count($address) == 4) {
                  $nodes[$i]['name'] = $address[0];
                  $nodes[$i]['street'] = $address[1];
                  $nodes[$i]['additional'] = $address[2];
                  $address[3] = preg_replace('/\<\/font\>/', '', $address[3]);
                  
                  $city_array = explode(',', $address[3]);
                  $city = $city_array[0];
                  $nodes[$i]['city'] = $city;
                  preg_match('/([A-Z]{2})/', $city_array[1], $matches);
                  $nodes[$i]['state'] = $matches[0];
                  preg_match('/(\d{5})/', $city_array[1], $matches);
                  $nodes[$i]['postal_code'] = $matches[0];
                }
                //drush_print_r($nodes[$i]);
                break;
     
      
            }
          }
        }
        

        $i++;
  
      }
      //drush_print_r($nodes);
      $this->nodes = $nodes;
    }
    
    //drush_print_r($nodes);
    
    return $nodes;
  }
  
  /**
   * Return the nodes.
   */
  public function getNumRows() {
    
    $this->nodes = array();
    $this->nodes = $this->getNodes();
    
    return count($this->nodes);
  }  
}