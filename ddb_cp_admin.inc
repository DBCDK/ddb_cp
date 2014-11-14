<?php

/**
 *
 * @file
 * Form code for the DDB CMS Control Panel.
 *
 */

function ddb_cp_menu_content() {
  $content = array();

  $link['access'] = 1;
  $link['href']   = 'admin/ddb_cp/config';
  $link['title']  = t('Configuration');
  $link['description'] = t('Configure this DDB instance.');
  $link['localized_options'] = array();

  $content['config'] = $link;

  if (ddb()->model() == 'webmaster') {
    $link['access'] = 1;
    $link['href']   = 'admin/ddb_cp/development';
    $link['title']  = t('Development');
    $link['description'] = t('Manage development of this DDB instance.');
    $link['localized_options'] = array();
  
    $content['dev'] = $link;
  }

  return theme('admin_block_content', array('content' => $content));
}


function ddb_cp_menu_configuration() {
  $form['ddb_config'] = array(
    '#type' => 'fieldset',
    '#collapsible' => 0,
    '#title' => t('Configuration'),
  );
  
  $form['ddb_config']['model'] = array(
    '#type' => 'item',
    '#markup' => t("You are currently subscribed to the '@model' model.", array('@model' => ddb()->model())),
  );

  $form['ddb_config']['servers'] = array(
    '#type' => 'fieldset',
    '#collapsible' => 1,
    '#title' => t('Server Addresses'),
  );

  $form['ddb_config']['servers']['ddb_service_url'] = array(
    '#type' => 'textfield',
    '#title' => t('DDB webservice URL'),
    '#default_value' => variable_get('ddb_service_url'),
  );

  $form['save_config'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#submit' => array('ddb_cp_config_submit'),
  );

  return $form;
}


function ddb_cp_config_submit($form, &$form_state) {
  variable_set('ddb_service_url', $form_state['values']['ddb_service_url']);
  
  cache_clear_all('ddb', 'cache', TRUE);
  
  drupal_set_message(t('Your configuration has been saved.'));
}


function ddb_cp_menu_development($form, &$form_state) {
  drupal_add_css(drupal_get_path('module', 'ddb_cp') . '/ddb_cp.css', array('group' => CSS_DEFAULT, 'type' => 'file'));

  $form += ddb_cp_form_development_dev($form, $form_state);
  $form += ddb_cp_form_development_stg($form, $form_state);
  $form += ddb_cp_form_development_prod($form, $form_state);
    
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'ddb_cp') . '/ddb_cp.js',
  );

  return $form;
}


function ddb_cp_form_development_dev($form, $form_state) {
  $form = array();
  $status = ddb()->status();
  
  if ($status['dev']['status'] != 'unknown' && $status['dev']['status'] != 'not_exist') {
    $form['create_dev'] = array(
      '#type' => 'fieldset',
      '#collapsible' => 1,
      '#title' => t('Developer Site'),
      '#description' => t('Your developer site can be found at <a href="@url">@url</a>.', array('@url' => url('http://' . ddb()->names['dev']))),
    );

    $form['create_dev']['url'] = array(
      '#type' => 'item',
      '#markup' => t('If you want to delete your current work and start over from scratch, you can do so by clicking the button below. Clicking the button will remove the staging site as well if one has been created.'),
    );

    $form['create_dev']['go-delete-dev'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#disabled' => 1,
      '#attributes' => array('onclick' => 'return (false);'),
    );

    $form['create_dev']['dev_status'] = array(
      '#type' => 'fieldset',
      '#collapsible' => 1,
      '#title' => t('Site status'),
      '#collapsed' => 1,
    );

    $form['create_dev']['dev_status']['ball'] = array(
      '#markup' => _ddb_cp_get_site_status_message('dev', $status['dev']),
    );

    $form['create_dev']['test'] = array(
      '#type' => 'fieldset',
      '#collapsed' => 1,
      '#collapsible' => 1,
      '#title' => t('Test status'),
    );

    $form['create_dev']['test']['ball'] = array(
      '#markup' => _ddb_cp_get_test_status_message('dev', $status['dev']),
    );

    $form['create_dev']['test']['go-test-dev'] = array(
      '#type' => 'submit',
      '#value' => t('Run new test'),
      '#attributes' => array('onclick' => 'return (false);'),
    );

    $form['create_dev']['test']['dev-result'] = array(
      '#type' => 'fieldset',
      '#collapsed' => 1,
      '#collapsible' => 1,
      '#title' => t('Last test result'),
    );

    $form['create_dev']['test']['dev-result']['text'] = array(
      '#markup' => ' ',
      '#prefix' => '<div id="dev-test-report">',
      '#suffix' => '</div>',
    );
  }
  else {
    $options = array();
    $default = NULL;
    $templates = NULL;
    
    if ($templates = ddb()->templates(ddb()->names['prod'])) {
      foreach ($templates as $template) {
        if ($template->{'enabled'} == 'true') {
          $options[$template->{'path'}] = $template->{'name'};
          if ($template->{'default'} == 'true') $default = $template->{'path'};
        }
      }
    }
  
    $form['create_dev'] = array(
      '#type' => 'fieldset',
      '#collapsible' => 1,
      '#title' => t('Developer Site'),
      '#description' => t('Currently you do not have a developer site.'),
    );

    $form['create_dev']['dev_url'] = array(
      '#type' => 'item',
      '#markup' =>  t('If you want to initialize a new delevopment flow, you can create a development site by clicking the button below.'),
    );
    
    $form['create_dev']['plat_frame'] = array(
      '#type' => 'fieldset',
      '#collapsible' => 0,
    );
    
    $form['create_dev']['plat_frame']['platform'] = array(
      '#type' => 'radios',
      '#title' => t('Platform'),
      '#options' => $options,
      '#description' => t('Choose the platform to use as the basis of the new developersite.'),
      '#default_value' => $default,
    );

    $form['create_dev']['go_create_dev'] = array(
      '#type' => 'submit',
      '#name' => 'create-dev',
      '#value' => t('Create'),
      '#submit' => array('ddb_cp_create_dev_submit'),
    );
  }

  return $form;
}


function ddb_cp_form_development_stg($form, $form_state) {
  $form = array();
  $status = ddb()->status();
  $install_time = date('j/n Y G:i', variable_get('install_time'));
  
  if ($status['stg']['status'] != 'unknown' && $status['stg']['status'] != 'not_exist') {
    $form['create_stg'] = array(
      '#type' => 'fieldset',
      '#collapsible' => 1,
      '#title' => t('Staging Site'),
      '#description' => t('Your staging site can be found at <a href="@url">@url</a>.', array('@url' => url('http://' . ddb()->names['stg']))),
      '#collapsed' => 1,
    );
    
    $form['create_stg']['info'] = array(
      '#type' => 'item',
      '#markup' => 'It was created at ' . $install_time . '. Accordingly, the staging site does not reflect any changes you may done on your developer site since then. You can update your staging site to include the latest development by pressing the button below.',
    );

    $form['create_stg']['go-update-stg'] = array(
      '#type' => 'submit',
      '#name' => 'update-stg',
      '#value' => t('Update'),
      '#attributes' => array('onclick' => 'return (false);'),
    );

    $form['create_stg']['stg-status'] = array(
      '#type' => 'fieldset',
      '#collapsible' => 1,
      '#title' => t('Site status'),
    );

    $form['create_stg']['stg-status']['ball'] = array(
      '#markup' => _ddb_cp_get_site_status_message('stg', $status['stg']),
    );

    $form['create_stg']['stg-test'] = array(
      '#type' => 'fieldset',
      '#collapsed' => 1,
      '#collapsible' => 1,
      '#title' => t('Test status'),
    );

    $form['create_stg']['stg-test']['ball'] = array(
      '#markup' => _ddb_cp_get_test_status_message('stg', $status['stg']),
    );

    $form['create_stg']['stg-test']['go-test-stg'] = array(
      '#type' => 'submit',
      '#value' => t('Run new test'),
      '#attributes' => array('onclick' => 'return (false);'),
    );

    $form['create_stg']['stg-test']['stg-result'] = array(
      '#type' => 'fieldset',
      '#collapsed' => 1,
      '#collapsible' => 1,
      '#title' => t('Last test result'),
    );

    $form['create_stg']['stg-test']['stg-result']['text'] = array(
      '#markup' => ' ',
      '#prefix' => '<div id="stg-test-report">',
      '#suffix' => '</div>',
    );
  }
  else {
    $form['create_stg'] = array(
      '#type' => 'fieldset',
      '#collapsed' => 1,
      '#collapsible' => 1,
      '#title' => t('Staging Site'),
      '#description' => t('Currently you do not have a staging site.'),
    );
  
    $form['create_stg']['stg_url'] = array(
      '#type' => 'item',
      '#markup' => t('The staging site is used for final testing of your development before moving on to production. You can only create a staging site if you have a valid development site with an error free test. To create a new staging site, click the button below.'),
    );
  
    $form['create_stg']['go_create_stg'] = array(
      '#type' => 'submit',
      '#name' => 'create-stg',
      '#value' => t('Create'),
      '#submit' => array('ddb_cp_create_stg_submit'),
    );
  }

  return $form;
}


function ddb_cp_form_development_prod($form, $form_state) {
  global $base_url;
  $form = array();
  $status = ddb()->status();

  $form['update_prod'] = array(
    '#type' => 'fieldset',
    '#collapsed' => 1,
    '#collapsible' => 1,
    '#title' => t('Production Site'),
    '#description' => t('Your production site can be found at <a href="@url">@url</a>.', array('@url' => url($base_url))),
  );

  $form['update_prod']['prod_url'] = array(
    '#type' => 'item',
    '#markup' => t('You must have a valid staging site with an error free test to update your production site. When you click the update button, your production site will be updated with your developments and the development flow will be ended. This means that your development and staging sites will be deleted. You can always start a new flow to do further development.'),
  );

  $form['update_prod']['go_update_prod'] = array(
    '#type' => 'submit',
    '#name' => 'create-prod',
    '#value' => t('Update'),
    '#submit' => array('ddb_cp_update_prod_submit'),
  );
  
  $form['update_prod']['prod_status'] = array(
    '#type' => 'fieldset',
    '#collapsible' => 1,
    '#title' => t('Site status'),
  );

  $form['update_prod']['prod_status']['prod_ball'] = array(
    '#markup' => _ddb_cp_get_site_status_message('prod', $status['prod']),
  );

  return $form;
}