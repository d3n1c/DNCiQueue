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

function dnciqueue_use_admin_params(array $form = array(), array $form_state = array()) {
  $defaults = variable_get('dnciqueue_use_params', array());
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
  $form['global']['agent[inputNumber]'] = array(
    '#type' => !empty($readonly) ? 'item' : 'select',
    '#title' => t('Input Next Number'),
    '#description' => !empty($readonly) ? NULL : t('Input new next number to serve'),
    '#options' => $options,
    '#default_value' => !empty($readonly) ? NULL : (!empty($defaults['agents']['inputNumber']) ? $defaults['agents']['inputNumber'] : key($options)),
    '#markup' => !empty($readonly) ? (!empty($defaults['agents']['inputNumber']) ? $defaults['agents']['inputNumber'] : NULL) : NULL,
  );
  $form['global']['submitg'] = array(
    '#type' => 'submit',
    '#value' => !empty($readonly) ? t('update') : t('save'),
    '#submit' => array('dnciqueue_use_agent_params_submit'),
  );
  unset ($options, $readonly);
  
  if (module_exists('dnciqueue')) {
    $getSection = dnciqueue_get_sections();
  }
  else {
    $getSection = array();
  }
//  echo '<pre>'; print_r($getSection); echo '</pre>'; exit;
  if (empty($form_state['storage']['agents'])) {
    if (!empty($getSection) && count($getSection) > 0) {
      $options = array();
      foreach ($getSection as $key => $value) {
        $options[$key] = $value['title'];
      }
      $form['fsection'] = array(
        '#type' => 'fieldset',
        '#title' => t('Section'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      $form['fsection']['section'] = array(
        '#type' => 'select',
        '#title' => t('Choose Section'),
        '#description' => t('Section that is part served by this system'),
        '#options' => $options,
        '#default_value' => !empty($defaults['section']) ? $defaults['section'] : key($options),
      );
      $form['fsection']['submits'] = array(
        '#type' => 'submit',
        '#value' => t('save'),
        '#submit' => array('dnciqueue_use_section_params_submit'),
      );
    }
    unset ($getSection);
  
    $form['reset'] = array(
      '#type' => 'fieldset',
      '#title' => t('Reset'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['reset']['clear'] = array(
      '#type' => 'submit',
      '#value' => t('Reset all queue!'),
      '#submit' => array('dncqueue_use_reset_all_queue'),
    );
  }
  
  return $form;
}

function dnciqueue_use_admin_params_submit(&$form, &$form_state) {
//  echo '<pre>'; print_r($form_state['values']); echo '</pre>'; exit;
  if (!empty($form_state['values']['clearconfirm'])) {
    variable_del('dnciqueue_use_numbers');
    drupal_set_message(t('All queue numbers have been clear successfully :)'));
  }
  $form_state['redirect'] = current_path();
}

function dnciqueue_use_section_params_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  $default = variable_get('dnciqueue_use_params', array());
  $default['section'] = $form_state['values']['section'];
  variable_set('dnciqueue_use_params', $default);
  unset($default);
  drupal_set_message(t('Configuration Section Parameters have been saved successfully :)'));
  $form_state['redirect'] = 'admin/config/system/dncsystem/dnciqueue/use';
}

function dnciqueue_use_agent_params_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  if (empty($form_state['values']['agents'])) {
    if (module_exists('dncclient')) {
      $agents = dncclient_get_all_agents();
      if (is_null($agents)) {
        unset ($agents);
        drupal_set_message(t('You need to make at least one agent here'), 'warning', FALSE);
        drupal_goto(drupal_get_path_alias('admin/structure/dncsystem/dncclient/agent/add'));
        return;
      }
    }
    if (is_array($agents) && count($agents) > 0) {
      foreach ($agents as $value) {
        if (module_exists('dnciqueue')) {
          $resources = dnciqueue_get_remote_resource($value['id_key'], $value['url']);
        }
        else {
          $resources = array();
        }
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
    $form_state['rebuild'] = TRUE;
    return;
  }
  
  $defaults = variable_get('dnciqueue_use_params', array());
  $defaults['agents'] = $form_state['input']['agent'];
  variable_set('dnciqueue_use_params', $defaults);
  unset ($defaults);
  drupal_set_message(t('Configuration Agents Parameters have been saved successfully :)'));
  $form_state['redirect'] = current_path();
}

function dncqueue_use_reset_all_queue(&$form, &$form_state) {
  if (empty($form_state['values']['clearconfirm'])) {
    $form_state['storage']['clearconfirm'] = TRUE;
    $form_state['rebuild'] = TRUE;
    return;
  }
}

function dnciqueue_use_admin_counters(array $form = array(), array $form_state = array()) {
  $defaults = dnciqueue_use_challange_global_vars();
//  echo '<pre>'; print_r($defaults); echo '</pre>'; exit;
  if (!empty($form_state['storage']['confirm'])) {
    foreach ($form_state['storage'] as $key => $value) {
      $form[$key] = array(
        '#type' => 'value',
        '#value' => $value,
      );
    }
    $description = t('It will delete all these items') . "\n" . '<ul>';
    foreach ($form_state['storage']['items'] as $value) {
      $user = user_load($value);
      $description .= '<li>' . $defaults['counters'][$value] . ' ' . t('by') . ' ' . $user->name . '</li>';
      unset ($user);
    }
    $description .= "\n" . '</ul>';
    return confirm_form(
            $form,
            t('Are you sure to delete all these items ?!'),
            'admin/structure/dncsystem/dnciqueueuse',
            $description,
            t('Yes sure!'),
            t('cancel')
           );
  }
  $header = array(
    'name' => array(
      'data' => t('Counter Name'),
      'width' => '65%',
    ),
    'user' => array(
      'data' => t('User'),
      'width' => '30%'
    ),
  );
  $options = array();
  if (!empty($defaults['counters']) && count($defaults['counters']) > 0) {
    foreach ($defaults['counters'] as $key => $value) {
      $user = user_load($key);
      $options[$key] = array(
        'name' => $value,
        'user' => $user->name,
      );
      unset ($user);
    }
    $form['submit'] = array(
      '#weight' => 100,
      '#type' => 'submit',
      '#value' => t('delete'),
    );
  }
  $form['items'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#empty' => t('No data available'),
    '#attributes' => array('width' => '100%'),
    '#caption' => t('Counters'),
    '#multiple' => TRUE,
  );
  unset ($header, $options);
  return $form;
}

function dnciqueue_use_admin_counters_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  if (empty($form_state['values']['confirm'])) {
    $items = array();
    foreach ($form_state['values']['items'] as $value) {
      if (!empty($value)) {
        $items[] = $value;
      }
    }
    if (count($items) > 0) {
      $form_state['storage']['items'] = $items;
      $form_state['storage']['confirm'] = 1;
    }
    unset ($items);
    $form_state['rebuild'] = TRUE;
    return;
  }
  $form_state['redirect'] = 'admin/structure/dncsystem/dnciqueueuse';
}

function dnciqueue_use_add_admin_counters(array $form = array(), array $form_state = array()) {
  return dnciqueue_use_admin_counters_form($form, $form_state);
}

function dnciqueue_use_admin_counters_form(array $form = array(), array $form_state = array()) {
  $defaults = dnciqueue_use_challange_global_vars();
  if (!empty($form_state['storage'])) {
    foreach ($form_state['storage'] as $key => $value) {
      $form[$key] = array(
        '#type' => 'value',
        '#value' => $value,
      );
    }
  }
  if (module_exists('dncmain')) {
    $options = dncmain_get_option_list_of_users();
  }
  else {
    $options = array();
  }
  unset ($options[1]);
  if (!empty($form_state['storage']['confirm'])) {
    $description = t('Update Counter @counter by @clerk', array('@counter' => $form_state['storage']['counter'], '@clerk' => $options[$form_state['storage']['user']]));
    return confirm_form(
            $form,
            t('Are you sure to update this counter ?!'),
            'admin/structure/dncsystem/dnciqueueuse',
            $description,
            t('Yes sure!'),
            t('cancel')
          );
  }
  if (!empty($defaults['counters'])) {
    foreach ($defaults['counters'] as $key => $value) {
      if (!empty($form_state['storage']['user']) && $key != $form_state['storage']['user']) {
        unset ($options[$key]);
      }
      else {
        unset ($options[$key]);
      }
    }
  }
  if (empty($form_state['storage']['user']) && count($options) < 2) {
    drupal_set_message(t('All users have been allocated as officer'), 'error', FALSE);
    drupal_goto('admin/structure/dncsystem/dnciqueueuse');
    return;
  }
  $form['user'] = array(
    '#type' => !empty($form_state['storage']['user']) ? 'item' : 'select',
    '#title' => t('User'),
    '#description' => t('Users who have the authority serve at the counter'),
    '#options' => $options,
    '#default_value' => !empty($form_state['storage']['user']) ? $form_state['storage']['user'] : key($options),
    '#markup' => !empty($form_state['storage']['user']) ? $options[$form_state['storage']['user']] : key($options),
    '#required' => TRUE,
    '#weight' => 10,
  );
  $form['counter'] = array(
    '#type' => !empty($form_state['storage']['justinfo']) ? 'item' : 'textfield',
    '#title' => t('Counter'),
    '#description' => t('Counter which will be handled by the user'),
    '#size' => 30,
    '#default_value' => !empty($form_state['storage']['counter']) ? $form_state['storage']['counter'] : NULL,
    '#markup' => !empty($form_state['storage']['counter']) ? $form_state['storage']['counter'] : NULL,
    '#required' => TRUE,
    '#weight' => 5,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => !empty($form_state['storage']['justinfo']) ? t('update') : t('save'),
    '#weight' => 100,
  );
  return $form;
}

function dnciqueue_use_admin_counters_form_validate(&$form, &$form_state) {
  if (empty($form_state['values']['user'])) {
    form_set_error('user', t('Please select user to be clerk of the counter'));
  }
}

function dnciqueue_use_admin_counters_form_submit(&$form, &$form_state) {
  form_state_values_clean($form_state);
  foreach ($form_state['values'] as $key => $value) {
    $form_state['storage'][$key] = $value;
  }
  if (!empty($form_state['values']['justinfo'])) {
    unset($form_state['storage']['justinfo']);
    $form_state['rebuild'] = TRUE;
    return;
  }
  if (empty($form_state['values']['confirm'])) {
    $form_state['storage']['confirm'] = 1;
    $form_state['rebuild'] = TRUE;
    return;
  }
  
  $defaults = dnciqueue_use_challange_global_vars();
  $defaults['counters'][$form_state['values']['user']] = $form_state['values']['counter'];
  variable_set('dnciqueue_use_params', $defaults);
  unset ($defaults);
}

function dnciqueue_use_add_admin_counters_validate(&$form, &$form_state) {
  dnciqueue_use_admin_counters_form_validate($form, $form_state);
}

function dnciqueue_use_add_admin_counters_submit(&$form, &$form_state) {
  dnciqueue_use_admin_counters_form_submit($form, $form_state);
  if (!empty($form_state['values']['justinfo']) || empty($form_state['values']['confirm'])) {
    return;
  }
  drupal_set_message(t('Addition of new counter clerk and has been successful'));
  $form_state['redirect'] = 'admin/structure/dncsystem/dnciqueueuse';
}