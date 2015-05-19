<?php

/* 
 * Copyright (C) 2015 denic
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

function dnciqueue_main_admin_params(array $form = array(), array $form_state = array()) {
  $defaults = variable_get('dnciqueue_params', array());
  if (!empty($form_state['storage'])) {
    foreach ($form_state['storage'] as $key => $value) {
      $form[$key] = array(
        '#type' => 'value',
        '#value' => $value,
      );
    }
  }
  else {
    $readonly = 1;
    if (!empty($defaults['agents']) && count($defaults['agents']) > 0) {
      $dump = array();
      foreach ($defaults['agents'] as $key => $value) {
        $value = base64_decode($value);
        $value = explode('|', $value);
        $value = implode(', ', $value);
        $dump[$key] = $value;
      }
      $defaults['agents'] = $dump;
      unset ($dump);
    }
  }
  
  if (!empty($form_state['storage']['clearconfirm'])) {
    $form['agents'] = array(
      '#type' => 'value',
      '#value' => TRUE,
    );
    return confirm_form(
            $form,
            t('Are you sure to clear all queue numbers ?!'),
            current_path(),
            t('This action can not be undone!!!'),
            t('Yes, do it now!'),
            t('cancel')
          );
  }
  
  $form['global'] = array(
    '#type' => 'fieldset',
    '#title' => t('Global'),
    '#description' => t('Global parameters and configurations'),
    '#collapsible' => TRUE,
    '#collapsed' => !empty($readonly) ? TRUE : FALSE,
  );
  $options = !empty($readonly) ? array() : $form_state['storage']['agents'];
  $form['global']['agent[sections]'] = array(
    '#type' => !empty($readonly) ? 'item' : 'select',
    '#title' => t('Get Sections List'),
    '#description' => !empty($readonly) ? NULL : t('Get list of available sections on remote server'),
    '#options' => $options,
    '#default_value' => !empty($readonly) ? NULL : (!empty($defaults['agents']['sections']) ? $defaults['agents']['sections'] : key($options)),
    '#markup' => !empty($readonly) ? (!empty($defaults['agents']['sections']) ? $defaults['agents']['sections'] : NULL) : NULL,
  );
  $form['global']['submit'] = array(
    '#type' => 'submit',
    '#value' => !empty($readonly) ? t('update') : t('save'),
  );
  unset ($options, $readonly);
  
  if (empty($form_state['storage']['agents'])) {
    $form['reset'] = array(
      '#type' => 'fieldset',
      '#title' => t('Reset'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['reset']['clear'] = array(
      '#type' => 'submit',
      '#value' => t('Reset all queue!'),
      '#submit' => array('dncqueue_reset_all_queue'),
    );
  }
  
  return $form;
}

function dnciqueue_main_admin_params_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  if (empty($form_state['values']['agents'])) {
    $agents = dncclient_get_all_agents();
    if (is_null($agents)) {
      unset ($agents);
      drupal_set_message(t('You need to make at least one agent here'), 'warning', FALSE);
      drupal_goto(drupal_get_path_alias('admin/structure/dncsystem/dncclient/agent/add'));
      return;
    }
    if (is_array($agents) && count($agents) > 0) {
      foreach ($agents as $value) {
        $resources = dnciqueue_get_remote_resource($value['id_key'], $value['url']);
        if (!is_null($resources)) {
          foreach ($resources as $ky => $vl) {
            $key = base64_encode($value['id_key'] . '|' . $value['url'] . '|' . $ky);
            $form_state['storage']['agents'][$value['title']][$key] = $vl['title'] . ' (' . $vl['description'] . ')';
            unset ($key);
          }
        }
        unset ($resources);
      }
    }
    unset ($agents);
//    echo '<pre>'; print_r($form_state['storage']); echo '</pre>'; exit;
    $form_state['rebuild'] = TRUE;
    return;
  }
  
  if (!empty($form_state['values']['clearconfirm'])) {
    form_state_values_clean($form_state);
    variable_del('dnciqueue_numbers');
    drupal_set_message(t('All queue numbers have been clear successfully :)'));
  }
  else {
    $defaults = array(
      'agents' => $form_state['input']['agent']
    );
    variable_set('dnciqueue_params', $defaults);
    unset ($defaults);
    drupal_set_message(t('Configuration Parameters have been saved successfully :)'));
  }
  $form_state['redirect'] = current_path();
}

function dncqueue_reset_all_queue(&$form, &$form_state) {
  if (empty($form_state['values']['clearconfirm'])) {
    $form_state['storage']['clearconfirm'] = TRUE;
    $form_state['rebuild'] = TRUE;
    return;
  }
}